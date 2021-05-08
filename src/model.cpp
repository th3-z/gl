//
// Created by the_z on 05/05/2021.
//

#include "model.h"


Model::Model(const char* filename) {
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool res = loader.LoadBinaryFromFile(&this->model, &err, &warn, filename);
    if (!warn.empty())
        std::cout << "WARN: " << warn << std::endl;
    if (!err.empty())
        std::cout << "ERR: " << err << std::endl;
    if (!res)
        std::cout << "Failed to load glTF: " << filename << std::endl;

    this->bindModel();
}

Model::~Model() {
    //glDeleteBuffers(1, &this->VBO);
    //glDeleteBuffers(1, &this->EBO);
    glDeleteVertexArrays(1, &this->VAO);
}

void Model::render(Shader &shader) {
    glBindVertexArray(this->VAO);

    const tinygltf::Scene &scene = this->model.scenes[this->model.defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        this->drawModelNodes(this->model.nodes[scene.nodes[i]]);
    }

    glBindVertexArray(0);
}

void Model::drawModelNodes(tinygltf::Node &node) {
    if ((node.mesh >= 0) && (node.mesh < this->model.meshes.size())) {
        drawMesh(this->model.meshes[node.mesh]);
    }
    for (size_t i = 0; i < node.children.size(); i++) {
        drawModelNodes(this->model.nodes[node.children[i]]);
    }
}

void Model::drawMesh(tinygltf::Mesh &mesh) {
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = this->model.accessors[primitive.indices];

        glDrawElements(
            primitive.mode,
            indexAccessor.count,
            indexAccessor.componentType,
            (char *)NULL + (indexAccessor.byteOffset)
        );
    }
}

void Model::bindModel() {
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);

    const tinygltf::Scene &scene = this->model.scenes[this->model.defaultScene];
    for (int i = 0; i < scene.nodes.size(); ++i) {
        assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
        this->bindModelNodes(model.nodes[scene.nodes[i]]);
    }

    glBindVertexArray(0);
    for (int i = 0; i < this->VBOs.size(); ++i) {
        glDeleteBuffers(1, &this->VBOs[i]);
    }
}

void Model::bindModelNodes(tinygltf::Node &node) {
    if ((node.mesh >= 0) && (node.mesh < this->model.meshes.size())) {
        this->bindMesh(this->model.meshes[node.mesh]);
    }

    for (int i = 0; i < node.children.size(); i++) {
        assert((node.children[i] >= 0) && (node.children[i] < this->model.nodes.size()));
        bindModelNodes(this->model.nodes[node.children[i]]);
    }
}

void Model::bindMesh(tinygltf::Mesh &mesh) {
    for (int i = 0; i < this->model.bufferViews.size(); ++i) {
        const tinygltf::BufferView &bufferView = this->model.bufferViews[i];
        const tinygltf::Buffer &buffer = this->model.buffers[bufferView.buffer];

        unsigned int VBO;
        glGenBuffers(1, &VBO);
        this->VBOs[i] = VBO;
        glBindBuffer(bufferView.target, VBO);

        glBufferData(
            bufferView.target,
            bufferView.byteLength,
            &buffer.data.at(0) + bufferView.byteOffset,
            GL_STATIC_DRAW
        );
    }

    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = this->model.accessors[primitive.indices];

        for (auto &attrib : primitive.attributes) {
            tinygltf::Accessor accessor = this->model.accessors[attrib.second];
            int byteStride = accessor.ByteStride(this->model.bufferViews[accessor.bufferView]);
            glBindBuffer(GL_ARRAY_BUFFER, this->VBOs[accessor.bufferView]);

            int size = 1;
            if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                size = accessor.type;
            }

            int vaa = -1;
            if (attrib.first.compare("POSITION") == 0) vaa = 0;
            if (attrib.first.compare("NORMAL") == 0) vaa = 1;
            if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;

            glEnableVertexAttribArray(vaa);
            glVertexAttribPointer(
                vaa,
                size,
                accessor.componentType,
                accessor.normalized ? GL_TRUE : GL_FALSE,
                byteStride,
                (char *)NULL + (accessor.byteOffset)
            );
        }

        if (!this->model.textures.empty()) {
            tinygltf::Texture &tex = this->model.textures[0];
            if (tex.source > -1) {
                unsigned int texHandle;
                glGenTextures(1, &texHandle);

                tinygltf::Image &image = model.images[tex.source];

                glBindTexture(GL_TEXTURE_2D, texHandle);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                GLenum format = GL_RGBA;
                if (image.component == 1) {
                    format = GL_RED;
                } else if (image.component == 2) {
                    format = GL_RG;
                } else if (image.component == 3) {
                    format = GL_RGB;
                }

                GLenum type = image.bits == 16? GL_UNSIGNED_BYTE: GL_UNSIGNED_SHORT;
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RGBA,
                    image.width,
                    image.height,
                    0,
                    format,
                    type,
                    &image.image.at(0)
                );
            }
        }
    }
}
