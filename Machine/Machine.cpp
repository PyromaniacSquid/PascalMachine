// Machine.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "PascalTranslator.h"


int main()
{
	string path = "TestScript.txt";
	try {
		PascalTranslator machine(path);
		//machine.Print();
		if (machine.Translate()) {
			cout << "Program is correct" << endl;
		}
		else {
			cout << "Program is incorrect, read errors log" << endl;
		}
	}
	catch (exception ex) {
		cout << ex.what() << endl;;
	}
	return 0;
}

