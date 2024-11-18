/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tests.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/15 11:41:04 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/18 14:53:10 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iomanip>
#include "../include/ft_irc.hpp"

#define RED "\033[0;31m"
#define GRN "\033[0;32m"
#define RESET "\033[0m"

#define NEW_SECTION std::cout << "\n" << std::setw(50) << std::setfill('=') << "\n" << std::endl;
#define PRINT_VAR(x) std::cout << #x << ": " << x << std::endl;
#define SUCCESS(x, y) std::cout << GRN #x ": " y RESET << std::endl;
#define FAILURE(x, y) std::cout << RED #x ": " y RESET << std::endl;

int main ()
{
	NEW_SECTION

	{
		std::cout << "Testing hashSimple(std::sting& in_str) function:\n" << std::endl;

		std::string	password = "ABCDEFG";
		std::string	right_attempt = "ABCDEFG";
		std::string	wrong_attempt =  "aBCDEFG";

		PRINT_VAR(password)
		unsigned int hashed_pass = hashSimple(password);
		PRINT_VAR(hashed_pass)

		PRINT_VAR(right_attempt)
		unsigned int hashed_right = hashSimple(right_attempt);
		PRINT_VAR(hashed_right)

		PRINT_VAR(wrong_attempt)
		unsigned int hashed_wrong = hashSimple(wrong_attempt);
		PRINT_VAR(hashed_wrong)


		if (hashed_pass == hashed_right)
			SUCCESS(hashed_pass == hashed_right, "works")
		else
			FAILURE(hashed_pass == hashed_right, "didn't pass when it should!")

		if (hashed_pass != hashed_wrong)
			SUCCESS(hashed_pass != hashed_wrong, "works")
		else
			FAILURE(hashed_pass != hashed_wrong, "passed when it shouldn't!")


		std::cout << "\n\nTesting that string is zeroed after hashing:\n" << std::endl;

		std::string example = "some password123";
		std::cout << "Before hashing:" << std::endl;
		PRINT_VAR(example);
		hashSimple(example);

		std::cout << "After hashing:" << std::endl;
		bool is_zeroed = 1;
		for (size_t i = 0; i < example.size(); ++i)
			is_zeroed *= (example[i] == 0);

		if (is_zeroed)
			SUCCESS(example, "is zeroed")
		else
			FAILURE(example, "is not zeroed")
	}

	NEW_SECTION
}
