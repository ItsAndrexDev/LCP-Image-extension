#pragma once
#include "FileManipulator.hpp"

namespace LCFiles {

    LCFile::LCFile(std::string path, unsigned short correctMagicNumber) : magicNumber(correctMagicNumber) {
		std::cout << "LCFile constructor called for path: " << path << std::endl;
        readStreamBinary(path);
    }

    LCFile::~LCFile() {
		// Destructor
    }


    FileProperties generateFromPixelData(unsigned short width, unsigned short height,
        unsigned char channels, const unsigned char* data) {
        FileProperties props;
        props.magicNumber = 2568;
        props.width = width;
        props.height = height;

        const size_t size = width * height * channels;
        props.pixelData.resize(size);

        for (size_t i = 0; i < size; i += channels) {
            props.pixelData[i + 0] = data[i + 0];
            props.pixelData[i + 1] = data[i + 1];
            props.pixelData[i + 2] = data[i + 2];

            if (channels == 4) {
                props.pixelData[i + 3] = data[i + 3]; // preserve alpha
            }
        }

        return props;
    }

    LCError saveFromProperties(const std::string& path, const FileProperties& props) {
        namespace fs = std::filesystem;

        if (fs::exists(path))
            return LCError::AlreadyExists;

        std::ofstream out(path, std::ios::binary);
        if (!out)
            return LCError::WriteError;

        writeUint16LE(out, static_cast<uint16_t>(props.magicNumber));
        writeUint16LE(out, static_cast<uint16_t>(props.width));
        writeUint16LE(out, static_cast<uint16_t>(props.height));

        out.write(reinterpret_cast<const char*>(props.pixelData.data()), props.pixelData.size());

        return LCError::None;
    }

    void LCFile::readStreamBinary(const std::string& path) {
		std::cout << "ReadStreamBinary called for path: " << path << std::endl;
		std::ifstream in(path, std::ios::binary);

        if (!in) {
			std::cout << "Failed to open file: " << path << std::endl;
            errorStatus = LCError::FileNotFound;
            return;

        }
		std::cout << "File opened successfully: " << path << std::endl;
        uint16_t magic = readUint16LE(in);
        uint16_t width = readUint16LE(in);
        uint16_t height = readUint16LE(in);
        // 8-bit RGB
        std::vector<unsigned char> pixels(width * height * 3);
        in.read(reinterpret_cast<char*>(pixels.data()), pixels.size());

        fileProperties.magicNumber = magic;
        fileProperties.width = width;
        fileProperties.height = height;
        fileProperties.pixelData = std::move(pixels);

		std::cout << "Read LCP file: " << path << " ("
            << "Magic: " << fileProperties.magicNumber << ", "
            << "Width: " << fileProperties.width << ", "
			<< "Height: " << fileProperties.height << ")\n";
        if (fileProperties.magicNumber != magicNumber) {
            errorStatus = LCError::InvalidFormat;
            return;
        }
        errorStatus = LCError::None;
        return;
    }

} // namespace LCFiles
