# Компилятор и флаги
CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -O2
TARGET    = sysmon
SRCDIR    = src
OBJDIR    = build
SOURCES   = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS   = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
DEPS      = $(OBJECTS:.o=.d)

# Цель по умолчанию
.PHONY: all clean

all: $(TARGET)

# Правило сборки исполняемого файла
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Правило компиляции .cpp → .o с генерацией зависимостей
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Включение сгенерированных зависимостей
-include $(DEPS)

# Очистка
clean:
	rm -rf $(OBJDIR) $(TARGET)