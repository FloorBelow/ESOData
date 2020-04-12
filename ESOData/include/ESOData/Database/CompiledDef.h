#ifndef ESODATA_DATABASE_COMPILED_DEF_H
#define ESODATA_DATABASE_COMPILED_DEF_H

namespace esodata {
	class SerializationStream;

	class CompiledDef {
	public:
		CompiledDef();
		virtual ~CompiledDef();

		CompiledDef(const CompiledDef& other) = delete;
		CompiledDef &operator =(const CompiledDef& other) = delete;

		virtual unsigned int defIndex() const = 0;
		virtual unsigned int defVersion() const = 0;

		virtual void serialize(SerializationStream& stream) const = 0;
		virtual void deserialize(SerializationStream& stream) = 0;
	};
}

#endif
