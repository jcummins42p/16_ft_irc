/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tests.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/15 11:41:04 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/15 12:29:53 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iomanip>
#include "../include/ft_irc.hpp"

#define RED "\033[0;31m"
#define GRN "\033[0;32m"
#define RESET "\033[0m"

#define NEW_SECTION std::cout << "\n" << std::setw(50) << std::setfill('=') << "\n" << std::endl;
#define PRINT_VAR(x) std::cout << #x << ": " << x << std::endl;
#define SUCCESS(x, y) std::cout << GRN << #x << ": " y RESET << std::endl;
#define FAILURE(x, y) std::cout << RED << #x << " " y RESET << std::endl;

int main ()
{
	NEW_SECTION

	{
		std::cout << "Testing simpleHash(std::sting& in_str) function:\n" << std::endl;

		std::string	password = "ABCDEFG";
		std::string	right_attempt = "ABCDEFG";
		std::string	wrong_attempt =  "aBCDEFG";

		unsigned int hashed_pass = simpleHash(password);
		PRINT_VAR(hashed_pass)

		unsigned int hashed_right = simpleHash(right_attempt);
		PRINT_VAR(hashed_right)

		unsigned int hashed_wrong = simpleHash(wrong_attempt);
		PRINT_VAR(hashed_wrong)


		if (hashed_pass == hashed_right)
			SUCCESS(hashed_pass == hashed_right, "works")
		else
			FAILURE(hashed_pass == hashed_right, "didn't pass when it should!")

		if (hashed_pass != hashed_wrong)
			SUCCESS(hashed_pass != hashed_wrong, "works")
		else
			FAILURE(hashed_pass != hashed_wrong, "passed when it shouldn't!")


		std::cout << "\nTesting that string is cleaned after hashing:\n" << std::endl;

		std::string example = "example";
		std::cout << "Before hashing:" << std::endl;
		PRINT_VAR(example);
		simpleHash(example);

		bool is_zeroed = true;
		if (example)
			SUCCESS(example, "is empty")
		else
			FAILURE()
	}

}
