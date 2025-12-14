#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "src/FileManip/FileManipulator.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "src/Rendering/stb_image.h"

int MAGICBYTE = 2568; // 'LCv1'

// Check if a file is a PNG image
bool isImageFile(const std::string& path) {
    std::string ext;
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        ext = path.substr(dotPos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }
    return ext == "png";
}

// Convert wide string to std::string (UTF-8)
std::string ws2s(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    int argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc < 2) {
        MessageBoxA(NULL, "Usage: <program> <file_path>", "Error", MB_OK | MB_ICONERROR);
        LocalFree(argvW);
        return -1;
    }

    // Single declaration of inputPath
    std::string inputPath = ws2s(argvW[1]);
    LocalFree(argvW);

    if (isImageFile(inputPath)) {
        // Generate LCP from PNG
        int fWidth, fHeight, fChannels;
        unsigned char* fData = stbi_load(inputPath.c_str(), &fWidth, &fHeight, &fChannels, 3);
        if (!fData) {
            MessageBoxA(NULL, "Failed to load image.", "Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        LCFiles::FileProperties props = LCFiles::generateFromPixelData(fWidth, fHeight, 3, fData);
        LCFiles::LCError saveStatus = LCFiles::saveFromProperties("test.lcp", props);
        stbi_image_free(fData);

        if (saveStatus == LCFiles::LCError::AlreadyExists) {
            MessageBoxA(NULL, "File test.lcp already exists. Remove it before running.", "Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        if (saveStatus != LCFiles::LCError::None) {
            MessageBoxA(NULL, "Failed to save LCP file.", "Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        MessageBoxA(NULL, "Successfully generated LCP file: test.lcp", "Success", MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    else {
        // View LCP file
        LCFiles::LCFile lcFile(inputPath, MAGICBYTE);

        if (!glfwInit()) {
            MessageBoxA(NULL, "GLFW init failed.", "Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        GLFWwindow* window = glfwCreateWindow(900, 700, "LCP Tools", nullptr, nullptr);
        if (!window) {
            MessageBoxA(NULL, "Window creation failed.", "Error", MB_OK | MB_ICONERROR);
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            MessageBoxA(NULL, "GLEW init failed.", "Error", MB_OK | MB_ICONERROR);
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
}
