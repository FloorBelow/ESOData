#include "ESODefCompiler.h"

#include <sstream>

ESODefCompiler::ESODefCompiler(const std::filesystem::path& directiveDirectory) {

	for (const auto& entry : std::filesystem::recursive_directory_iterator(directiveDirectory)) {
		if (!entry.is_regular_file())
			continue;

		esodata::DatabaseDirectiveFile directives;
		directives.parseFile(entry);

		insertTypes<DefType>(directives.defs());
		insertTypes<StructureType>(directives.structures());
		insertTypes<EnumType>(directives.enums());
		insertTypes<DefAliasType>(directives.defAliases());
	}

	nestTypes();

	NestedTypeName path;
	processCollectionDependencies(m_types, path);
}

ESODefCompiler::~ESODefCompiler() = default;

void ESODefCompiler::nestTypes() {
	for (auto it = m_types.begin(); it != m_types.end(); ) {
		const auto& pair = *it;
		const auto& name = pair.first;

		bool removed = false;

		auto offset = name.find_last_of('_');
		if (offset != std::string::npos) {
			auto parent = name.substr(0, offset);
			auto nameInParent = name.substr(offset + 1);

			NestedTypeName nesting;
			auto& parentType = findTypeByName(parent, nesting);

			nesting.emplace_back(nameInParent);

			m_renames.emplace(name, std::move(nesting));
			parentType.nestedTypes.emplace(std::move(nameInParent), std::move(it->second));
			it = m_types.erase(it);

			removed = true;
		}

		if (!removed)
			++it;
	}
}

void ESODefCompiler::processCollectionDependencies(std::map<std::string, Type>& collection, NestedTypeName &path) {
	for (auto& entry : collection) {
		auto& data = entry.second;

		path.emplace_back(entry.first);

		std::visit([this, &data, &path](auto& typeData) {
			processEntryDependencies(data, path, typeData);
		}, data.data);
		
		processCollectionDependencies(data.nestedTypes, path);

		path.pop_back();
	}
}

void ESODefCompiler::processStructureFieldDependencies(Type& type, const NestedTypeName& path, const esodata::DatabaseDirectiveFile::Structure& structure) {
	for (const auto& entry : structure.fields) {
		if (entry.type == esodata::DatabaseDirectiveFile::FieldType::Enum ||
			entry.type == esodata::DatabaseDirectiveFile::FieldType::Struct ||
			entry.type == esodata::DatabaseDirectiveFile::FieldType::PolymorphicReference ||
			(entry.type == esodata::DatabaseDirectiveFile::FieldType::Array && (
				entry.arrayType == esodata::DatabaseDirectiveFile::FieldType::Enum ||
				entry.arrayType == esodata::DatabaseDirectiveFile::FieldType::Struct ||
				entry.arrayType == esodata::DatabaseDirectiveFile::FieldType::PolymorphicReference))) {

			addDependencyToType(type, path, entry.typeName);
		}
	}
}

void ESODefCompiler::processEntryDependencies(Type& type, const NestedTypeName& path, StructureType& structure) {
	processStructureFieldDependencies(type, path, structure.directive);
}

void ESODefCompiler::processEntryDependencies(Type& type, const NestedTypeName& path, DefType& defType) {
	addDependencyToType(type, path, "BaseDef");
	processStructureFieldDependencies(type, path, defType.directive);
}

void ESODefCompiler::processEntryDependencies(Type& type, const NestedTypeName& path, EnumType& enumType) {
	(void)type;
	(void)path;
	(void)enumType;
}

void ESODefCompiler::processEntryDependencies(Type& type, const NestedTypeName& path, DefAliasType& defAliasType) {
	addDependencyToType(type, path, defAliasType.directive.targetName);
}

void ESODefCompiler::addDependencyToType(Type& type, const NestedTypeName& path, const std::string& dependencyName) {
	NestedTypeName dependencyPath;
	auto& dependencyType = findTypeByName(dependencyName, dependencyPath);

	size_t sameParts = 0;
	auto* collection = &m_types;
	auto* deps = &m_rootDependencies;

	for (auto depFromIt = path.begin(), depToIt = dependencyPath.cbegin();
		depFromIt != path.end() && depToIt != dependencyPath.cend();
		++depFromIt, ++depToIt) {

		if (*depFromIt == *depToIt) {
			auto collectionIt = collection->find(*depFromIt);
			auto& type = collectionIt->second;

			collection = &type.nestedTypes;
			deps = &type.dependencies;

			sameParts++;
		}
		else {
			break;
		}
	}

	if (sameParts == path.size()) {
		// dependency from base type to nested type, no graph edge is required
	}
	else if (sameParts == dependencyPath.size()) {
		throw std::runtime_error("dependency between nested type and its outer type is not allowed: " + composeName(path) + " -> " + composeName(dependencyPath));
	}
	else {
		// unrelated types
		deps->emplace(path[sameParts], std::move(dependencyPath[sameParts]));
	}
}

std::string ESODefCompiler::composeName(const NestedTypeName& name) const {
	std::stringstream outputName;
	bool first = true;
	for (const auto& part : name) {
		if (first) {
			first = false;
		}
		else {
			outputName << "::";
		}

		outputName << part;
	}

	return outputName.str();
}

template<typename RecordType, typename T>
void ESODefCompiler::insertTypes(const std::vector<T>& typeSet) {
	for (const auto& type : typeSet) {
		auto result = m_types.emplace(type.name, Type{ RecordType { type } });
		if (!result.second)
			throw std::runtime_error("duplicate type name: " + type.name);
	}
}

auto ESODefCompiler::findTypeByName(const std::string& name, NestedTypeName& nesting) -> Type& {
	auto renameIt = m_renames.find(name);
	auto collection = &m_types;
	if (renameIt != m_renames.end()) {
		nesting = renameIt->second;
	}
	else {
		nesting = NestedTypeName{ name };
	}

	Type* entry = nullptr;

	for (const auto& name : nesting) {
		auto it = collection->find(name);
		if (it == collection->end()) {
			throw std::runtime_error("type was not found: " + name);
		}

		entry = &it->second;
		collection = &entry->nestedTypes;
	}

	return *entry;
}

void ESODefCompiler::generateSource(std::ostream& header, std::ostream& source, const std::string &headerRelativeToSource) {
	header <<
		"#ifndef GENERATED_ESODATA_PARSER_HEADER_INCLUDED\n"
		"#define GENERATED_ESODATA_PARSER_HEADER_INCLUDED\n"
		"\n"
		"#include <ESOData/Database/CompiledDef.h>\n"
		"#include <ESOData/Database/ForeignKey.h>\n"
		"#include <ESOData/Database/PolymorphicReference.h>\n"
		"#include <ESOData/Database/AssetReference.h>\n"
		"#include <ESOData/Serialization/SerializationStream.h>\n"
		"\n"
		"namespace esodata {\n"
		"\n";

	source <<
		"#include \"" << headerRelativeToSource << "\"\n"
		"#include <ESOData/Serialization/SizedVector.h>\n"
		"\n"
		"namespace esodata {\n";

	m_headerStream = &header;
	m_sourceStream = &source;
	
	generateForwardDeclarations(m_types);

	std::unordered_set<std::string> visitedTypes;
	generateTypes(m_types, m_rootDependencies, visitedTypes);

	header << m_endOfHeader.str();

	header <<
		"}\n"
		"\n"
		"#endif\n"
		"\n";

	source <<
		"}\n"
		"\n";
}

void ESODefCompiler::generateTypes(const std::map<std::string, Type>& types, const std::multimap<std::string, std::string>& dependencies, std::unordered_set<std::string>& visitedTypes) {
	for (const auto& type : types) {
		generateType(type.first, type.second, types, dependencies, visitedTypes);
	}
}

void ESODefCompiler::generateType(const std::string& typeName, const Type& type, const std::map<std::string, Type>& types, const std::multimap<std::string, std::string>& dependencies, std::unordered_set<std::string>& visitedTypes) {
	auto deps = dependencies.equal_range(typeName);
	for (auto depIt = deps.first; depIt != deps.second; ++depIt) {
		auto depTypeIt = types.find(depIt->second);
		if (depTypeIt == types.end())
			throw std::runtime_error("broken dependency");

		generateType(depTypeIt->first, depTypeIt->second, types, dependencies, visitedTypes);
	}

	auto visitedResult = visitedTypes.emplace(typeName);
	if (!visitedResult.second)
		return;

	m_currentType.push_back(typeName);

	std::visit([this, &typeName, &type](auto& data) {
		generateTypeHeading(typeName, type, data);
	}, type.data);

	m_currentOuterType.push_back(typeName);

	std::unordered_set<std::string> visitedNesteds;
	generateTypes(type.nestedTypes, type.dependencies, visitedNesteds);

	m_currentOuterType.pop_back();
	
	std::visit([this, &typeName, &type](auto& data) {
		generateTypeTrailer(typeName, type, data);
	}, type.data);

	m_currentType.pop_back();
}

void ESODefCompiler::generateTypeHeading(const std::string& typeName, const Type& type, const StructureType& structure) {
	indent(0);
	*m_headerStream << "struct " << typeName << " {\n";
}

void ESODefCompiler::generateTypeTrailer(const std::string& typeName, const Type& type, const StructureType& structure) {
	writeFields(structure.directive);

	indent(0);
	*m_headerStream << "};\n";

	auto fullName = composeName(m_currentType);
	m_endOfHeader <<
		"SerializationStream &operator <<(SerializationStream &stream, const struct " << fullName << "& value);\n"
		"SerializationStream &operator >>(SerializationStream &stream, struct " << fullName << "& value);\n"
		"\n";

	*m_sourceStream <<
		"  SerializationStream &operator <<(SerializationStream &stream, const struct " << fullName << "& value) {\n";

	writeSerializer(structure.directive, "<<", "value.");

	*m_sourceStream <<
		"    return stream;\n"
		"  }\n"
		"\n"
		"  SerializationStream &operator >>(SerializationStream &stream, struct " << fullName << "& value) {\n";

	writeSerializer(structure.directive, ">>", "value.");

	*m_sourceStream <<
		"    return stream;\n"
		"  }\n"
		"\n";
}

void ESODefCompiler::generateTypeHeading(const std::string& typeName, const Type& type, const DefType& def) {
	indent(0);
	*m_headerStream << "class " << typeName << " : public BaseDef, public CompiledDef {\n";
	indent(0);
	*m_headerStream << "public:\n";
}

void ESODefCompiler::writeDefAddressing(unsigned int index) {
	indent();
	*m_headerStream << "static constexpr unsigned int DefIndex = " << index << ";\n";

	indent();
	*m_headerStream << "unsigned int defIndex() const override;\n";

	auto fullName = composeName(m_currentType);

	*m_sourceStream <<
		"  unsigned int " << fullName << "::defIndex() const {\n"
		"    return DefIndex;\n"
		"  }\n"
		"\n";
}

void ESODefCompiler::generateTypeTrailer(const std::string& typeName, const Type& type, const DefType& def) {
	indent();
	*m_headerStream << typeName << "();\n";

	indent();
	*m_headerStream << "~" << typeName << "() override;\n";

	writeDefAddressing(def.directive.defIndex);

	indent();
	*m_headerStream << "static constexpr unsigned int DefVersion = " << def.directive.version << ";\n";

	indent();
	*m_headerStream << "unsigned int defVersion() const override;\n";

	indent();
	*m_headerStream << "void serialize(SerializationStream &stream) const override;\n";

	indent();
	*m_headerStream << "void deserialize(SerializationStream &stream) override;\n";

	writeFields(def.directive);

	indent(0);
	*m_headerStream << "};\n";

	auto fullName = composeName(m_currentType);
	*m_sourceStream <<
		"  " << fullName << "::" << typeName << "() = default;\n"
		"\n"
		"  " << fullName << "::~" << typeName << "() = default;\n"
		"\n"
		"  unsigned int " << fullName << "::defVersion() const {\n"
		"    return DefVersion;\n"
		"  }\n"
		"\n"
		"  void " << fullName << "::serialize(SerializationStream &stream) const {\n"
	    "    stream << static_cast<const BaseDef &>(*this);\n";

	writeSerializer(def.directive, "<<", "");

	*m_sourceStream <<
		"  }\n"
		"\n"
		"  void " << fullName << "::deserialize(SerializationStream &stream) {\n"
		"    stream >> static_cast<BaseDef &>(*this);\n";

	writeSerializer(def.directive, ">>", "");

	*m_sourceStream <<
		"  }\n"
		"\n";
}

void ESODefCompiler::writeSerializer(const esodata::DatabaseDirectiveFile::Structure& structure, const std::string& op, const std::string& selfRef) {
	unsigned int index = 0;

	for (const auto& field : structure.fields) {
		*m_sourceStream << "    " << "stream " << op << " ";
		
		if (field.type == esodata::DatabaseDirectiveFile::FieldType::Array) {
			*m_sourceStream << "makeSizedVector<uint32_t>(";
		}

		*m_sourceStream << selfRef;

		if (field.name.empty()) {
			*m_sourceStream << "unnamed" << index;
		}
		else {
			*m_sourceStream << field.name;
		}

		if (field.type == esodata::DatabaseDirectiveFile::FieldType::Array) {
			*m_sourceStream << ")";
		}

		*m_sourceStream << ";\n";

		index += 1;
	}
}

void ESODefCompiler::generateTypeHeading(const std::string& typeName, const Type& type, const EnumType& enumType) {
	indent(0);
	*m_headerStream << "enum class " << typeName << " {\n";
}

void ESODefCompiler::generateTypeTrailer(const std::string& typeName, const Type& type, const EnumType& enumType) {
	for (const auto& value : enumType.directive.values) {
		std::string name;

		auto nameIt = enumType.directive.valueNames.find(value);
		if (nameIt == enumType.directive.valueNames.end()) {
			name = "Unnamed";

			if (value < 0) {
				name += "M" + std::to_string(-value);
			}
			else {
				name += std::to_string(value);
			}
		}
		else {
			name = nameIt->second;
		}

		for (auto& ch : name) {
			if (ch == '$' || ch == '%') {
				ch = '_';
			}
		}

		if (name == "NULL") {
			name = "PolymorphicNull";
		}

		indent();
		*m_headerStream << name << " = " << value << ",\n";
	}

	indent(0);
	*m_headerStream << "};\n";
}

void ESODefCompiler::generateTypeHeading(const std::string& typeName, const Type& type, const DefAliasType& defAlias) {
	indent(0);
	*m_headerStream << "class " << typeName << " : public " << getCxxNameFor(defAlias.directive.targetName) << " {\n";
}

std::string ESODefCompiler::getCxxNameFor(const std::string& name) {
	NestedTypeName targetPath;
	const auto& target = findTypeByName(name, targetPath);

	return composeName(targetPath);
}

void ESODefCompiler::generateTypeTrailer(const std::string& typeName, const Type& type, const DefAliasType& defAlias) {
	writeDefAddressing(defAlias.directive.defIndex);

	indent(0);
	*m_headerStream << "};\n";
}

void ESODefCompiler::indent(int adjust) {
	for (int ch = 0, indent = 2 * (1 + m_currentOuterType.size() + adjust); ch < indent; ch++) {
		*m_headerStream << ' ';
	}
}

void ESODefCompiler::writeFields(const esodata::DatabaseDirectiveFile::Structure& structure) {
	unsigned int index = 0;

	for (const auto& field : structure.fields) {
		indent();
		writeFieldType(field.type, field);

		*m_headerStream << " ";

		if (field.name.empty()) {
			*m_headerStream << "unnamed" << index;
		}
		else {
			*m_headerStream << field.name;
		}
		
		*m_headerStream << ";\n";

		index += 1;
	}
}

void ESODefCompiler::writeFieldType(esodata::DatabaseDirectiveFile::FieldType type, const esodata::DatabaseDirectiveFile::StructureField& field) {
	switch (type) {
	case esodata::DatabaseDirectiveFile::FieldType::Int8:
		*m_headerStream << "int8_t";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::Int16:
		*m_headerStream << "int16_t";

		break;	
	
	case esodata::DatabaseDirectiveFile::FieldType::Int32:
		*m_headerStream << "int32_t";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::Int64:
		*m_headerStream << "int64_t";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::UInt8:
		*m_headerStream << "uint8_t";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::UInt16:
		*m_headerStream << "uint16_t";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::UInt32:
		*m_headerStream << "uint32_t";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::UInt64:
		*m_headerStream << "uint64_t";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::Float:
		*m_headerStream << "float";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::Enum:
		*m_headerStream << getCxxNameFor(field.typeName);
		break;

	case esodata::DatabaseDirectiveFile::FieldType::String:
		*m_headerStream << "std::string";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::Array:
		*m_headerStream << "std::vector<";
		writeFieldType(field.arrayType, field);
		*m_headerStream << ">";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::ForeignKey:
		*m_headerStream << "ForeignKey<" << getCxxNameFor(field.typeName) << ">";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::Boolean:
		*m_headerStream << "bool";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::Struct:
		*m_headerStream << "struct " << getCxxNameFor(field.typeName);
		break;

	case esodata::DatabaseDirectiveFile::FieldType::PolymorphicReference:
		*m_headerStream << "PolymorphicReference<" << getCxxNameFor(field.typeName) << ">";
		break;

	case esodata::DatabaseDirectiveFile::FieldType::AssetReference:
		*m_headerStream << "AssetReference";
		break;
	}
}

void ESODefCompiler::generateForwardDeclarations(const std::map<std::string, Type>& types) {
	for (const auto& pair : types) {
		generateForwardDeclaration(pair.first, pair.second);
	}
}

void ESODefCompiler::generateForwardDeclaration(const std::string& typeName, const Type& type) {
	std::visit([typeName, type, this](auto& data) {
		generateForwardDeclaration(typeName, type, data);
	}, type.data);
}

void ESODefCompiler::generateForwardDeclaration(const std::string& typeName, const Type& type, const StructureType& structure) {
	(void)typeName;
	(void)type;
	(void)structure;
}

void ESODefCompiler::generateForwardDeclaration(const std::string& typeName, const Type& type, const DefType& def) {
	(void)type;
	(void)def;

	*m_headerStream << "  class " << typeName << ";\n";
}

void ESODefCompiler::generateForwardDeclaration(const std::string& typeName, const Type& type, const EnumType& enumType) {
	(void)typeName;
	(void)type;
	(void)enumType;
}

void ESODefCompiler::generateForwardDeclaration(const std::string& typeName, const Type& type, const DefAliasType& defAlias) {
	(void)type;
	(void)defAlias;

	*m_headerStream << "  class " << typeName << ";\n";
}
