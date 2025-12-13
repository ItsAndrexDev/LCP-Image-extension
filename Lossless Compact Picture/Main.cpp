#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "src/FileManip/FileManipulator.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "src/Rendering/stb_image.h"

int MAGICBYTE = 2568; // 'LCv1'

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage:\n";
        std::cout << "  " << argv[0] << " <image_path> generate   - to generate LCP from image\n";
        std::cout << "  " << argv[0] << " <lcp_path> view        - to view LCP image\n";
        return 0;
    }

    std::string inputPath = argv[2];
    std::string command = argv[1];

    if (command == "generate") {
        int fWidth, fHeight, fChannels;
        unsigned char* fData = stbi_load(inputPath.c_str(), &fWidth, &fHeight, &fChannels, 3); // force 3 channels

        if (!fData) {
            std::cerr << "Failed to load image: " << inputPath << "\n";
            return -1;
        }

        LCFiles::FileProperties props = LCFiles::generateFromPixelData(fWidth, fHeight, 3, fData);
        LCFiles::LCError saveStatus = LCFiles::saveFromProperties("test.lcp", props);

        if (saveStatus != LCFiles::LCError::None && saveStatus != LCFiles::LCError::AlreadyExists) {
            std::cerr << "Failed to save LCP file\n";
            stbi_image_free(fData);
            return -1;
        }

        std::cout << "Successfully generated LCP file.\n";
        stbi_image_free(fData);
        return 0;
    }
    else if (command == "view") {
        LCFiles::LCFile lcFile(inputPath, MAGICBYTE);
        std::cout << "Loaded image: " << lcFile.getWidth() << "x" << lcFile.getHeight() << "\n";

        if (!glfwInit()) {
            std::cerr << "GLFW init failed\n";
            return -1;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        GLFWwindow* window = glfwCreateWindow(900, 700, "CPU Image Viewer", nullptr, nullptr);
        if (!window) {
            std::cerr << "Window creation failed\n";
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "GLEW init failed\n";
            glfwTerminate();
            return -1;
        }

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            int winWidth, winHeight;
            glfwGetFramebufferSize(window, &winWidth, &winHeight);
            glViewport(0, 0, winWidth, winHeight);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, winWidth, 0, winHeight, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            float scaleX = float(winWidth) / lcFile.getWidth();
            float scaleY = float(winHeight) / lcFile.getHeight();
            float scale = std::min(scaleX, scaleY);

            float imgWidth = lcFile.getWidth() * scale;
            float imgHeight = lcFile.getHeight() * scale;
            float offsetX = (winWidth - imgWidth) / 2.0f;
            float offsetY = (winHeight - imgHeight) / 2.0f;

            GLuint tex;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lcFile.getWidth(), lcFile.getHeight(), 0,
                GL_RGB, GL_UNSIGNED_BYTE, lcFile.getPixelData().data());

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, tex);

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(offsetX, offsetY);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(offsetX + imgWidth, offsetY);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(offsetX + imgWidth, offsetY + imgHeight);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(offsetX, offsetY + imgHeight);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            glDeleteTextures(1, &tex);

            glfwSwapBuffers(window);
        }

        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
    else {
        std::cerr << "Unknown command: " << command << "\n";
        return -1;
    }
}
