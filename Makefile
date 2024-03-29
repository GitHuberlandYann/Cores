NAME			= cores
OBJS_DIR		= Objs
SRCS_DIR		= Sources

FILES			= main
FILES			+= Render/callbacks Render/Display Render/Gui Render/Text
FILES			+= UDP/Address UDP/Socket
FILES			+= Utils/random Utils/utils

SRCS			= $(addprefix $(SRCS_DIR)/, $(addsuffix .cpp, $(FILES)))
OBJS 			= $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(FILES)))

# ===---===---===---===---===---===---===---===---===---===---===---===---

CC 			= clang++
CPPFLAGS 	= -Wall -Wextra -Werror -O3 -std=c++17
SAN 		= -fsanitize=address -g3
INCLUDES	= -I Includes -I Libs/glfw/include -I Libs/SOIL/build/include -I Libs/glew-2.2.0/include
LDFLAGS		= Libs/glfw/src/libglfw3.a Libs/glew-2.2.0/build/lib/libGLEW.a Libs/SOIL/build/lib/libSOIL.a

# ===---===---===---===---===---===---===---===---===---===---===---===---

ifeq ($(shell uname), Linux)
LDFLAGS		+= -lGL -lX11 -lpthread -lXrandr -lXi -ldl
else
LDFLAGS		+= -framework OpenGl -framework AppKit -framework IOkit
endif

# # ===---===---===---===---===---===---===---===---===---===---===---===---

all: $(OBJS_DIR) $(NAME)

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(OBJS_DIR)/Render
	@mkdir -p $(OBJS_DIR)/UDP
	@mkdir -p $(OBJS_DIR)/Utils

setup:
	cd Libs/SOIL && ./configure && make
	cd Libs/glfw && cmake . && make
	cd Libs/glew-2.2.0/build && cmake ./cmake && make

cleanLibs:
	cd Libs/SOIL && make clean
	cd Libs/glew-2.2.0 && make clean
	cd Libs/glfw && make clean

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) $(OBJS) -o $(NAME) $(LDFLAGS)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

rer: re
	@./$(NAME)

.PHONY: all setup cleanLibs clean fclean re rer
