CXX ?= g++
CXXFLAGS ?= -g
CPPFLAGS += -Iinclude -Iglm
LDLIBS += -lglfw -lGL -lGLU

TARGET := projetoCG
SOURCES := main.cpp SphereCube.cpp ImgIO.cpp glad.c Camera.cpp Renderer.cpp Shader.cpp lodepng.cpp
HEADERS := include/SphereCube.h include/ImgIO.h include/Shader.h include/glad/glad.h include/KHR/khrplatform.h include/Camera.h include/lodepng.h include/CImg.h include/Renderer.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): Makefile $(SOURCES) $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o $@ $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
