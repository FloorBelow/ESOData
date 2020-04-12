#ifndef ESO_DEF_COMPILER_H
#define ESO_DEF_COMPILER_H

#include <filesystem>
#include <map>
#include <variant>
#include <ios>
#include <string>
#include <unordered_set>
#include <sstream>

#include <ESOData/Directives/DatabaseDirectiveFile.h>

class ESODefCompiler {
public:
	explicit ESODefCompiler(const std::filesystem::path& directiveDirectory);
	~ESODefCompiler();

	ESODefCompiler(const ESODefCompiler& other) = delete;
	ESODefCompiler &operator =(const ESODefCompiler& other) = delete;

	void generateSource(std::ostream& header, std::ostream& source, const std::string &headerRelativeToSource);

private:
	struct StructureType {
		esodata::DatabaseDirectiveFile::Structure directive;
	};

	struct DefType {
		esodata::DatabaseDirectiveFile::Structure directive;
	};

	struct EnumType {
		esodata::DatabaseDirectiveFile::Enum directive;
	};

	struct DefAliasType {
		esodata::DatabaseDirectiveFile::DefAlias directive;
	};

	struct Type {
		std::variant<StructureType, DefType, EnumType, DefAliasType> data;
		std::map<std::string, Type> nestedTypes;
		std::multimap<std::string, std::string> dependencies;
	};

	using NestedTypeName = std::vector<std::string>;

	template<typename RecordType, typename T>
	void insertTypes(const std::vector<T> &typeSet);

	Type& findTypeByName(const std::string& name, NestedTypeName& nesting);

	void nestTypes();
	void processCollectionDependencies(std::map<std::string, Type>& collection, NestedTypeName &path);
	void processEntryDependencies(Type& type, const NestedTypeName& path, StructureType& structure);
	void processEntryDependencies(Type& type, const NestedTypeName& path, DefType& defType);
	void processEntryDependencies(Type& type, const NestedTypeName& path, EnumType &enumType);
	void processEntryDependencies(Type& type, const NestedTypeName& path, DefAliasType &defAliasType);
	void addDependencyToType(Type& type, const NestedTypeName& path, const std::string& dependencyName);
	
	void processStructureFieldDependencies(Type& type, const NestedTypeName& path, const esodata::DatabaseDirectiveFile::Structure& structure);

	void generateTypes(const std::map<std::string, Type>& types, const std::multimap<std::string, std::string>& dependencies, std::unordered_set<std::string>& visitedTypes);
	void generateType(const std::string& typeName, const Type& type, const std::map<std::string, Type>& types, const std::multimap<std::string, std::string>& dependencies, std::unordered_set<std::string>& visitedTypes);

	void generateTypeHeading(const std::string &typeName, const Type &type, const StructureType& structure);
	void generateTypeHeading(const std::string& typeName, const Type& type, const DefType& def);
	void generateTypeHeading(const std::string& typeName, const Type& type, const EnumType& enumType);
	void generateTypeHeading(const std::string& typeName, const Type& type, const DefAliasType& defAlias);
	void generateTypeTrailer(const std::string& typeName, const Type& type, const StructureType& structure);
	void generateTypeTrailer(const std::string& typeName, const Type& type, const DefType& def);
	void generateTypeTrailer(const std::string& typeName, const Type& type, const EnumType& enumType);
	void generateTypeTrailer(const std::string& typeName, const Type& type, const DefAliasType& defAlias);

	void indent(int adjust = 1);

	std::string composeName(const NestedTypeName& name) const;

	void writeDefAddressing(unsigned int index);
	void writeFields(const esodata::DatabaseDirectiveFile::Structure& structure);
	void writeFieldType(esodata::DatabaseDirectiveFile::FieldType type, const esodata::DatabaseDirectiveFile::StructureField& field);
	std::string getCxxNameFor(const std::string& name);
	void writeSerializer(const esodata::DatabaseDirectiveFile::Structure& structure, const std::string& op, const std::string& selfRef);;

	void generateForwardDeclarations(const std::map<std::string, Type>& types);
	void generateForwardDeclaration(const std::string &typeName, const Type &type);
	void generateForwardDeclaration(const std::string& typeName, const Type& type, const StructureType &structure);
	void generateForwardDeclaration(const std::string& typeName, const Type& type, const DefType& def);
	void generateForwardDeclaration(const std::string& typeName, const Type& type, const EnumType& enumType);
	void generateForwardDeclaration(const std::string& typeName, const Type& type, const DefAliasType& defAlias);

	std::map<std::string, Type> m_types;
	std::map<std::string, NestedTypeName> m_renames;
	std::multimap<std::string, std::string> m_rootDependencies;
	std::ostream* m_headerStream;
	std::ostream* m_sourceStream;
	std::stringstream m_endOfHeader;
	NestedTypeName m_currentOuterType;
	NestedTypeName m_currentType;
};

#endif
