# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmakagon <mmakagon@student.42.com>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/11/04 14:17:14 by jcummins          #+#    #+#              #
#    Updated: 2024/12/04 13:18:53 by jcummins         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME = ircserver

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

SRCS = main.cpp Channel.cpp Client.cpp hashSimple.cpp Server.cpp simpleHash.cpp
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
HEADS = ft_irc.hpp Channel.hpp Client.hpp Server.hpp
MAKE = ./Makefile

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Wunused-result -pedantic -std=c++98 \
		   -g -fsanitize=address -Iinclude

all: $(NAME)

$(NAME): $(OBJS)
	@echo "✅ Linking object files into executable $@:"
	@$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(addprefix $(INC_DIR)/, $(HEADS)) $(MAKE)
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
