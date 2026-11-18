CXX ?= g++
CXXFLAGS ?= -g
CPPFLAGS += -Iinclude -Iglm
LDLIBS += -lglfw -lGL -lGLU

TARGET := projetoCG
SOURCES := main.cpp SphereCube.cpp glad.c Camera.cpp
HEADERS := include/SphereCube.h include/glad/glad.h include/KHR/khrplatform.h include/Camera.h include/stb_image.h include/stb_image_write.h include/CImg.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): Makefile $(SOURCES) $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o $@ $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
