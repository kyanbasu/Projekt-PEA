CXX = clang++
CXXFLAGS = -std=c++17 -Wall -O2

TARGET = lista1
SOURCES = ./src/*.cpp ./include/*.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

debug: $(SOURCES)
	clang++ $(CXXFLAGS) -Werror -g -fsanitize=address $(SOURCES) -o $(TARGET).exe

.PHONY: clean
clean:
ifeq ($(OS),Windows_NT)
	powershell -Command "Remove-Item -Path ./$(TARGET)* -ErrorAction SilentlyContinue"
else
	rm -f $(TARGET)*
endif