#include <ESOData/Serialization/DeflatedSegment.h>

#include <zlib.h>

#include <stdexcept>

namespace esodata {

	static void *zlibAlloc(void *opaque, unsigned int items, unsigned int size) {
		(void)opaque;

		return malloc(items * size);
	}

	static void zlibFree(void *opaque, void *ptr) {
		(void)opaque;

		free(ptr);
	}

	std::vector<unsigned char> zlibCompress(const unsigned char *inputData, size_t inputLength) {
		struct ManagedStream : z_stream {
			ManagedStream() {
				zalloc = zlibAlloc;
				zfree = zlibFree;
				opaque = nullptr;

				int result = deflateInit(this, Z_BEST_COMPRESSION);
				if (result != Z_OK)
					throw std::runtime_error("zlib error");
			}

			~ManagedStream() {
				deflateEnd(this);
			}
		} stream;

		std::vector<unsigned char> output(deflateBound(&stream, inputLength));

		stream.next_in = inputData;
		stream.avail_in = inputLength;
		stream.next_out = output.data();
		stream.avail_out = output.size();

		int result = deflate(&stream, Z_FINISH);

		if (result != Z_STREAM_END || stream.avail_in != 0)
			throw std::runtime_error("zlib error");

		output.resize(stream.total_out);
		output.shrink_to_fit();

		return output;
	}

	void zlibUncompress(const unsigned char *inputData, size_t inputLength, unsigned char *outputData, size_t outputLength) {
		struct ManagedStream : z_stream {
			ManagedStream() {
				zalloc = zlibAlloc;
				zfree = zlibFree;
				opaque = nullptr;

				int result = inflateInit(this);
				if (result != Z_OK)
					throw std::runtime_error("zlib error");
			}

			~ManagedStream() {
				inflateEnd(this);
			}
		} stream;

		stream.next_in = inputData;
		stream.avail_in = inputLength;
		stream.next_out = outputData;
		stream.avail_out = outputLength;

		int result = inflate(&stream, Z_FINISH);

		if(result != Z_STREAM_END || stream.avail_in != 0 || stream.avail_out != 0)
			throw std::runtime_error("zlib error");
	}

}