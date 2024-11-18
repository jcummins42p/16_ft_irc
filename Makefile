# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmakagon <mmakagon@student.42.com>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/11/04 14:17:14 by jcummins          #+#    #+#              #
#    Updated: 2024/11/18 18:21:28 by mmakagon         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME = ircserver

SRC_DIR = src
OBJ_DIR = obj

SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
INCLUDES = -I./include

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Wunused-result -pedantic -std=c++98 $(INCLUDES) -fsanitize=address

all: $(NAME)

$(NAME): $(OBJS)
	@echo "✅ Linking object files into executable $@:"
	@$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "✅ Compiling object file $@ from source file $<"
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "✅ Removing all object files and dir"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "✅ Removing executable"
	@rm -f $(NAME)

re: fclean all
