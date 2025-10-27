#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace core {
    Texture::Texture(const std::string &path) {
        glGenTextures(1, &id);
        int width, height, nrComponents;
        if (unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0)) {
            GLenum format = 0;
            if (nrComponents == 1) {
                format = GL_RED;
            } else if (nrComponents == 3) {
                format = GL_RGB;
            } else if (nrComponents == 4) {
                format = GL_RGBA;
            }

            printf("Loaded with %d x %d [Components: %d]!\r\n", width, height, nrComponents);
            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0,
                         static_cast<GLint>(format)
                    , width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(data);
        } else {
            printf("Texture failed to load at path: %s\n", path.c_str());
            stbi_image_free(data);
        }
    }

    GLuint Texture::getId() {
        return id;
    }
}