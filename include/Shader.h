#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <glad/glad.h>
#include "glm/glm.hpp"

class Shader
{
    public:
        GLuint ID = 0;
        Shader(const std::string& vertexPath, const std::string& fragmentPath);
        void use() const;
        void setMat4(const std::string& name, const glm::mat4& mat) const;
        void setVec3(const std::string& name, const glm::vec3& value) const;
        void setFloat(const std::string& name, const float value) const;
        ~Shader();

    
    private:
        void checkCompilation(GLuint shader, std::string type);


};