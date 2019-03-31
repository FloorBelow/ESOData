#ifndef ESODATA_SERIALIZATION_OUTPUT_SERIALIZATION_STREAM_H
#define ESODATA_SERIALIZATION_OUTPUT_SERIALIZATION_STREAM_H

#include <ESOData/Serialization/SerializationStream.h>

#include <vector>

namespace esodata {
	class OutputSerializationStream final : public SerializationStream {
	public:
		OutputSerializationStream();
		explicit OutputSerializationStream(SerializationStream *otherStream);
		~OutputSerializationStream();

		virtual unsigned char *getRegionForWrite(size_t size) override;
		virtual const unsigned char *getRegionForRead(size_t size) override;

		virtual size_t getCurrentPosition() const override;
		virtual void setCurrentPosition(size_t position) override;
		
		inline std::vector<unsigned char> &&data() { return std::move(m_data); }

	private:
		std::vector<unsigned char> m_data;
		SerializationStream *m_targetStream;
		size_t m_position;
		size_t m_offset;
	};
}

#endif
