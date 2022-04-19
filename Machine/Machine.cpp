// Machine.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "PascalTranslator.h"


int main()
{
	string path = "TestScript.txt";
	try {
		PascalTranslator machine(path);
		machine.Print();
	}
	catch (exception ex) {
		cout << ex.what() << endl;;
	}
	return 0;
}

