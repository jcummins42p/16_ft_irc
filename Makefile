/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Makefile                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-04 13:10:27 by pyerima           #+#    #+#             */
/*   Updated: 2024-11-04 13:10:27 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

NAME = ircserv

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
INCLUDES = -I./include

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 $(INCLUDES)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
