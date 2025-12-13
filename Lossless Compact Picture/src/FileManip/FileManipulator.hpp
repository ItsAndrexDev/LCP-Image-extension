#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include <string>
#include <filesystem>
#include "../Rendering/stb_image.h"

namespace LCFiles {

    enum class LCError {
        None,
        FileNotFound,
        InvalidFormat,
        UnsupportedVersion,
        ReadError,
        WriteError,
        AlreadyExists
    };

    inline void writeUint16LE(std::ofstream& out, uint16_t value) {
        unsigned char bytes[2];
        bytes[0] = value & 0xFF;         // LSB
        bytes[1] = (value >> 8) & 0xFF;  // MSB
        out.write(reinterpret_cast<char*>(bytes), 2);
    }

    inline uint16_t readUint16LE(std::ifstream& in) {
        unsigned char bytes[2];
        in.read(reinterpret_cast<char*>(bytes), 2);
        return uint16_t(bytes[0]) | (uint16_t(bytes[1]) << 8);
    }

    struct FileProperties {
        int magicNumber;
        int width;
        int height;
        std::vector<unsigned char> pixelData; // 8-bit RGB
    };

    FileProperties generateFromPixelData(int width, int height, int channels, const unsigned char* data);
    LCError saveFromProperties(const std::string& path, const FileProperties& props);

    struct LCFile {
        LCFile(std::string path, int correctMagicNumber = 2568);
        //~LCFile();

        inline unsigned short getWidth() const { return fileProperties.width; }
        inline unsigned short getHeight() const { return fileProperties.height; }
        inline const std::vector<unsigned char>& getPixelData() const { return fileProperties.pixelData; }
        inline LCError getErrorStatus() { return errorStatus; }

        LCError readStreamBinary(const std::string& path);

    private:
        FileProperties fileProperties;
        LCError errorStatus = LCError::None;
    };

} // namespace LCFiles
