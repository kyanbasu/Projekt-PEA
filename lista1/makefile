CXX = clang++
CXXFLAGS = -std=c++17 -Wall -O2 -I./include

TARGET = lista1
SOURCES = ./src/*.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET).out

debug: $(SOURCES)
	$(CXX) $(CXXFLAGS) -Werror -g -fsanitize=address $(SOURCES) -o $(TARGET).out

.PHONY: clean
clean:
ifeq ($(OS),Windows_NT)
	powershell -Command "Remove-Item -Path ./$(TARGET)* -ErrorAction SilentlyContinue"
else
	rm -f $(TARGET)*
endif