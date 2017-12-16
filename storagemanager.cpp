

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
#define NUMBER_OF_TYPES_IN_A_PAGE 9
#define TYPE_SIZE 223
#define MAX_FIELD_NAME 12
#define MAX_FIELD_NUMBER 16
int showOptions();
int createType();
int deleteType();
int listTypes();
int createRecord();
int deleteRecord();
int updateRecord();
int searchRecord();
int listRecords();
//teplate for type metadata in system catalog
string typeTemplate = "1$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";

//struct for a type metadata
struct RecordType{
	bool isDeleted;
	string typeName;
	int numberOfFields = 0;
	string fieldNames[16];
	string keyField;
	
};

//struct for the record
struct Record {
	int* fieldValues ;
	bool isDeleted;
};

//creates an empty page for system catalog file with given offset(page id)
int createEmptyPageCat(fstream& sysCatFS, int offset) {
	sysCatFS.seekg(offset, sysCatFS.beg);
	sysCatFS << typeTemplate ;
	sysCatFS << typeTemplate ;
	sysCatFS << typeTemplate ;
	sysCatFS << typeTemplate ;
	sysCatFS << typeTemplate ;
	sysCatFS << typeTemplate ;
	sysCatFS << typeTemplate ;
	sysCatFS << typeTemplate ;
	sysCatFS << typeTemplate;
	sysCatFS << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";

	return 0;
}

//creates system catalog file when the program starts
int createSysCatFile() {
	fstream sysCat;
	sysCat.open("systemcatalogue.txt", ios::in | ios::out);

	if (!sysCat.is_open()) {
		sysCat.close();
		sysCat.open("systemcatalogue.txt", ios::out);
		createEmptyPageCat(sysCat, 0);
		sysCat.close();

	}
	else
		sysCat.close();

	return 0;
}

//creates an empty page for a type file(.dat) with given page id and field number
int createEmptyDataPage(fstream& dataFS,  int pageID,int numberOfFields) {
	bool isFull = false;
	bool isDeleted = true;
	int numberOfRecords = 0;
	int numberOfRecordsInPage = 2039 / (sizeof(int) * numberOfFields + 1);
	//write default page header with id
	dataFS.seekg(pageID*2048, dataFS.beg);
	dataFS.write((char*)&pageID, sizeof(int));
	dataFS.seekg(pageID*2048+4, dataFS.beg);
	dataFS.write((char*)&numberOfRecords, sizeof(int));
	dataFS.seekg(pageID*2048 + 8, dataFS.beg);
	dataFS.write((char*)&isFull, sizeof(bool));
	for (int i = 0; i < numberOfRecordsInPage; i++) {
		dataFS.write((char*)&isDeleted, sizeof(bool));
		for (int j = 0; j < numberOfFields; j++) {
			dataFS.write((char*)&numberOfRecords, sizeof(int));
		}
	}
	for (int i = 0; i < 2039 % (sizeof(int) * numberOfFields + 1); i++) {
		dataFS.write((char*)&isFull, sizeof(bool));
	}
	return 0;
}

int main()
{
	int choice;
	createSysCatFile();
	while (1) {
		showOptions();
		cin >> choice;
		
		switch (choice)
		{
		case 1:
			createType();
			break;
		case 2:
			deleteType();
			break;
		case 3:
			listTypes();
			break;
		case 4:
			createRecord();
			break;
		case 5:
			deleteRecord();
			break;
		case 6:
			updateRecord();
			break;
		case 7:
			searchRecord();
			break;
		case 8:
			listRecords();
			break;
		}
		if (choice < 1 || choice > 8) {
			
			cout << "Choose a valid option" << endl ;
			cout << "Press a key to proceed" << endl << endl << endl;
			getchar(); getchar();
		}
	}

}
int showOptions() {
	cout << "Database Management System" << endl << endl;
	cout << "DDL Operations:" << endl;
	cout << "1. Create a Type" << endl;
	cout << "2. Delete a Type" << endl;
	cout << "3. List All Types" << endl << endl;
	cout << "DML Operations" << endl;
	cout << "4. Create a Record" << endl;
	cout << "5. Delete a Record" << endl;
	cout << "6. Update a Record" << endl;
	cout << "7. Search for a Record" << endl;
	cout << "8. List All Records of a Type" << endl << endl;
	return 0;
}


int createType() {
	RecordType newType;
	newType.isDeleted = false;
	cout << "Enter type name :" ;
	cin >> newType.typeName;
	cout << "Enter Number of Fields: ";
	cin >> newType.numberOfFields;
	cout << "Enter Field Names "<<endl;
	for (int i = 0; i < newType.numberOfFields; i++) {
		cout << i << ". Field Name: ";
		cin >> newType.fieldNames[i];
	}
	cout << "Enter a Correct Key Field Name: ";
	cin >> newType.keyField;
	cout<<endl<<endl;
	fstream sysCat;
	sysCat.open("systemcatalogue.txt", ios::in | ios::out);

	sysCat.seekg(0, sysCat.end);
	int numberOfPages = sysCat.tellg() / 2048;
	sysCat.seekg(0, sysCat.beg);
	//buffer for current page
	char* currentPage = new char[2048];
	//page id starts from 0
	int firstEmptyPageID = numberOfPages;
	string pageData;
	string currentType;
	RecordType currentTypeObj;
	//check for duplicate type or find an empty spot
	for (int i = 0; i < numberOfPages; i++) {
		sysCat.read(currentPage, 2048);
		pageData = string(currentPage);
		//iterate over 8 types in the page and parse over a string
		for (int j = 0; j < NUMBER_OF_TYPES_IN_A_PAGE; j++) {
			size_t endOfString = 16;
			currentType = pageData.substr(j*TYPE_SIZE , TYPE_SIZE );
			if (currentType[0] == '0') {
				currentTypeObj.isDeleted = false;
			}
			else {
				currentTypeObj.isDeleted = true;
			}
			//check if there is a deleted type(empty space)
			if (!currentTypeObj.isDeleted) {
				currentTypeObj.typeName = currentType.substr(1, 16);
				//extract the type name by finding the $ sign
				endOfString = currentTypeObj.typeName.find("$");
				currentTypeObj.typeName = currentTypeObj.typeName.substr(0, endOfString);
				//if duplicate return to main menu
				if (currentTypeObj.typeName.compare(newType.typeName) == 0) {
					sysCat.close();
					cout << "Duplicate type returning to menu." << endl;
					cout << "Press a key to proceed" << endl;
					getchar(); getchar();
					return 1;
				}
			}
			else {
				//if it is deleted write on this type 
				//if there is no deleted in between, the default type has a isDeleted ==1 so this is also works appending to end of the file
				sysCat.seekp(2048 * i + j*TYPE_SIZE);	//go to free subblock
				sysCat << newType.isDeleted;
				sysCat << newType.typeName;
				//fill spaces with $
				for (int z = 0; z < 16 - newType.typeName.length(); z++) {
					sysCat << "$";
				}
				sysCat.seekp(2048 * i + j*TYPE_SIZE+17);
				sysCat << newType.numberOfFields;
				//check if number of fields is 1 or 2 digits
				if (newType.numberOfFields / 10 == 0)
					sysCat << "$";
				//write type names
				for (int z = 0; z < newType.numberOfFields; z++) {
					sysCat << newType.fieldNames[z];
					//fill spaces with $
					for (int t = 0; t < MAX_FIELD_NAME - newType.fieldNames[z].length(); t++) {
						sysCat << "$";
					}
				}
				//fill remaining type name areas with $
				for (int z = 0; z < MAX_FIELD_NUMBER - newType.numberOfFields; z++) {
					sysCat << "$$$$$$$$$$$$";
				}
				sysCat << newType.keyField;
				for (int z = 0; z < MAX_FIELD_NAME - newType.keyField.length(); z++) {
					sysCat << "$";
				}
				fstream dataFile;
				dataFile.open(newType.typeName + ".dat", ios::out);
				dataFile.close();
				dataFile.open(newType.typeName + ".dat", ios::out|ios::in);
				createEmptyDataPage(dataFile,0,newType.numberOfFields);
				cout << "Type created!" << endl;
				cout << "Press a key to proceed" << endl<<endl;
				getchar(); getchar();
				dataFile.close();
				sysCat.close();
				return 0;
			}
		}
	}
	//if we reached here it means all pages are full
	//create new page to system catalog file
	createEmptyPageCat(sysCat, 2048 * firstEmptyPageID);
	sysCat.seekp(2048 * firstEmptyPageID);
	sysCat << newType.isDeleted;
	sysCat << newType.typeName;
	//fill spaces with $
	for (int z = 0; z < 16 - newType.typeName.length(); z++) {
		sysCat << "$";
	}
	sysCat.seekp(2048 * firstEmptyPageID + 17);
	sysCat << newType.numberOfFields;
	//check if number of fields is 1 or 2 digits
	if (newType.numberOfFields / 10 == 0)
		sysCat << "$";
	//write type names
	for (int z = 0; z < newType.numberOfFields; z++) {
		sysCat << newType.fieldNames[z];
		//fill spaces with $
		for (int t = 0; t < MAX_FIELD_NAME - newType.fieldNames[z].length(); t++) { 
			sysCat << "$";
		}
	}
	//fill remaining type name areas with $
	for (int z = 0; z < MAX_FIELD_NUMBER - newType.numberOfFields; z++) {
		sysCat << "$$$$$$$$$$$$";
	}
	sysCat << newType.keyField;
	for (int z = 0; z < MAX_FIELD_NAME - newType.keyField.length(); z++) {
		sysCat << "$";
	}

	//create a new file for that type
	fstream dataFile;
	dataFile.open(newType.typeName + ".dat", ios::out);
	dataFile.close();
	dataFile.open(newType.typeName + ".dat", ios::out | ios::in);
	createEmptyDataPage(dataFile,0,newType.numberOfFields); 
	cout << "Type created!" << endl;
	cout << "Press a key to proceed" << endl << endl;
	getchar(); getchar();
	dataFile.close();
	sysCat.close();
	return 0;
}
int deleteType() {
	fstream sysCat;
	sysCat.open("systemcatalogue.txt", ios::in | ios::out);
	sysCat.seekg(0, sysCat.end);
	int numberOfPages = (sysCat.tellg() / 2048);
	sysCat.seekg(0, sysCat.beg);
	//buffer for current page
	char* currentPage = new char[2048];
	string pageData;
	string currentType;
	RecordType currentTypeObj;
	string typeDeleted;
	cout << "Enter type name to be deleted: ";
	cin >> typeDeleted;
	for (int i = 0; i < numberOfPages; i++) {
		sysCat.read(currentPage, 2048);
		pageData = string(currentPage);
		//iterate over 9 types in the page and parse over a string
		for (int j = 0; j < NUMBER_OF_TYPES_IN_A_PAGE; j++) {
			size_t endOfString = 16;
			currentType = pageData.substr(j*TYPE_SIZE, TYPE_SIZE);
			if (currentType[0] == '0') {
				currentTypeObj.isDeleted = false;
			}
			else {
				currentTypeObj.isDeleted = true;
			}
			//check if there is a deleted type(empty space)
			if (!currentTypeObj.isDeleted) {
				currentTypeObj.typeName = currentType.substr(1, 16);
				//extract the type name by finding the $ sign
				endOfString = currentTypeObj.typeName.find("$");
				currentTypeObj.typeName = currentTypeObj.typeName.substr(0, endOfString);
				if (typeDeleted.compare(currentTypeObj.typeName) == 0) {
					char del = '1';
					sysCat.seekg(2048 * i + j*TYPE_SIZE, sysCat.beg);
					sysCat.write((char*)&del,sizeof(char));
					remove((typeDeleted + ".dat").c_str());
					cout << "Type deleted!" << endl;
					cout << "Press a key to proceed" << endl << endl;
					getchar(); getchar();
					sysCat.close();
					return 1;
				}
			}
		}
	}
	cout << "Type name is not found!" << endl;
	cout << "Press a key to proceed" << endl << endl;
	getchar(); getchar();
	sysCat.close();
	return 0;
}
int listTypes() {
	fstream sysCat;
	sysCat.open("systemcatalogue.txt", ios::in | ios::out);
	sysCat.seekg(0, sysCat.end);
	int numberOfPages = (sysCat.tellg() / 2048) ;
	sysCat.seekg(0, sysCat.beg);
	//buffer for current page
	char* currentPage = new char[2048];
	string pageData;
	string currentType;
	RecordType currentTypeObj;
	cout << "All types:" << endl<<endl;
	for (int i = 0; i < numberOfPages; i++) {
		sysCat.read(currentPage, 2048);
		pageData = string(currentPage);
		//iterate over 8 types in the page and parse over a string
		for (int j = 0; j < NUMBER_OF_TYPES_IN_A_PAGE; j++) {
			size_t endOfString = 16;
			currentType = pageData.substr(j*TYPE_SIZE, TYPE_SIZE);
			if (currentType[0] == '0') {
				currentTypeObj.isDeleted = false;
			}
			else {
				currentTypeObj.isDeleted = true;
			}
			//check if there is a deleted type(empty space)
			if (!currentTypeObj.isDeleted) {
				currentTypeObj.typeName = currentType.substr(1, 16);
				//extract the type name by finding the $ sign
				endOfString = currentTypeObj.typeName.find("$");
				currentTypeObj.typeName = currentTypeObj.typeName.substr(0, endOfString);
				cout << currentTypeObj.typeName << endl;
			}
		}
	}
	cout << endl<<"Press a key to proceed" << endl << endl;
	getchar(); getchar();
	return 0;
}
RecordType findType(fstream &sysCat, string typeName) {
	sysCat.seekg(0, sysCat.end);
	int numberOfPages = sysCat.tellg() / 2048;
	sysCat.seekg(0, sysCat.beg);
	//buffer for current page
	char* currentPage = new char[2048];
	string pageData;
	string currentType;
	RecordType currentTypeObj;
	for (int i = 0; i < numberOfPages; i++) {
		sysCat.read(currentPage, 2048);
		pageData = string(currentPage);
		//iterate over 8 types in the page and parse over a string
		for (int j = 0; j < NUMBER_OF_TYPES_IN_A_PAGE; j++) {
			size_t endOfString = 16;
			currentType = pageData.substr(j*TYPE_SIZE, TYPE_SIZE);
			if (currentType[0] == '0') {
				currentTypeObj.isDeleted = false;
			}
			else {
				currentTypeObj.isDeleted = true;
			}
			//check if there is a deleted type(empty space)
			if (!currentTypeObj.isDeleted) {
				currentTypeObj.typeName = currentType.substr(1, 16);
				//extract the type name by finding the $ sign
				endOfString = currentTypeObj.typeName.find("$");
				currentTypeObj.typeName = currentTypeObj.typeName.substr(0, endOfString);
				string numberFields = currentType.substr(17, 2);
				if (numberFields[1] == '$') {
					currentTypeObj.numberOfFields = numberFields[0] - '0';
				}
				else {
					currentTypeObj.numberOfFields = 10 * (numberFields[0] - '0');
					currentTypeObj.numberOfFields += numberFields[1] - '0';
				}
				//extract the type name by finding the $ sign
				for (int z = 0; z < currentTypeObj.numberOfFields; z++) {
					currentTypeObj.fieldNames[z] = currentType.substr(19+z*12, 12);
					endOfString = currentTypeObj.fieldNames[z].find("$");
					currentTypeObj.fieldNames[z] = currentTypeObj.fieldNames[z].substr(0, endOfString);
				}
				currentTypeObj.keyField = currentType.substr(19 + MAX_FIELD_NUMBER * 12, 12);
				endOfString = currentTypeObj.keyField.find("$");
				currentTypeObj.keyField = currentTypeObj.keyField.substr(0, endOfString);
				if (currentTypeObj.typeName.compare(typeName) == 0) {
					return currentTypeObj;
				}
			}
		}
	}
	RecordType dummyType;
	return dummyType;
}
int createRecord() {
	string typeName;
	cout << "Enter the type of the record to be created: ";
	cin >> typeName;
	fstream sysCat;
	sysCat.open("systemcatalogue.txt", ios::in | ios::out);
	fstream dataFile;
	dataFile.open(typeName + ".dat", ios::in | ios::out);
	RecordType type;
	Record record;
	type = findType(sysCat,typeName);
	if (type.numberOfFields == NULL) {
		cout << "No such type name found" << endl;
		return -1;
	}
	record.isDeleted = 0;
	
	vector<int> fieldValues;
	for (int i = 0; i < type.numberOfFields; i++) {
		int value;
		cout << "Enter value for field   " + type.fieldNames[i] << endl;
		cin >> value;
		fieldValues.push_back(value);
	}
	//we obtained the type and the values
	//we need to find an empty spot for the record
	//2048- pageheader(9) = 2039
	int numberOfRecordsInPage = 2039 / (sizeof(int) * type.numberOfFields + 1);
	dataFile.seekg(0, dataFile.end);
	int numberOfPages = (dataFile.tellg() / 2048);
	dataFile.seekg(0, dataFile.beg);
	bool isFull;
	bool isDeleted=true;
	int dummy = 0;
	bool dummybool;
	for (int t = 0; t < numberOfPages; t++) {
		isFull = false;
		dataFile.seekg(t*2048+8, dataFile.beg);
		dataFile.read((char*)&isFull, sizeof(bool));
		//if full go to next page
		if (isFull) {
			break;
		}
		for (int i = 0; i < numberOfRecordsInPage; i++) {
			dataFile.read((char*)&isDeleted, sizeof(bool));
			for (int k = 0; k < type.numberOfFields; k++) {
				dataFile.read((char*)&dummy, sizeof(int));
			}
			//if the record is deleted write new record to this block
			if (isDeleted) {
				dummybool = false;
				dataFile.seekg(t * 2048 + 9 + i*(sizeof(int)*type.numberOfFields + 1), dataFile.beg);
				dataFile.write((char*)&dummybool, sizeof(bool));
				for (int j = 0; j < type.numberOfFields; j++)
					dataFile.write((char*)&fieldValues.at(j), sizeof(int));

				cout << "record inserted" << endl<<endl<<endl;
				return 0;

			}
		

		}
		isFull = 1;
		dataFile.seekg(t * 2048 + 8, dataFile.beg);
		dataFile.write((char*)&isFull, sizeof(bool));
		
	}

	//if we reached here, it means there is no empty space in type file(.dat)
	//we create a new data page in that file and write the value to that page
	bool abc = false;
	int nuRecords = 0;
	createEmptyDataPage(dataFile, numberOfPages,type.numberOfFields);
	dataFile.seekg(numberOfPages * 2048 , dataFile.beg);
	dataFile.write((char*)&numberOfPages, sizeof(int));
	dataFile.write((char*)&nuRecords, sizeof(int));
	dataFile.write((char*)&abc, sizeof(bool));
	for (int j = 0; j < type.numberOfFields; j++)
		dataFile.write((char*)&fieldValues.at(j), sizeof(int));
	cout << "record inserted to empty page" << endl;
	return 0;
}

int deleteRecord() {
	string typeName;
	int keyValue;
	int recordIndex = 0;
	int recordPage = 0;
	cout << "Enter the type of the record to be deleted: ";
	cin >> typeName;

	fstream sysCat;
	sysCat.open("systemcatalogue.txt", ios::in | ios::out);
	fstream dataFile;
	dataFile.open(typeName + ".dat", ios::in | ios::out);
	RecordType type;
	Record record;
	type = findType(sysCat, typeName);
	cout << "Enter the key value  " + type.keyField + " :";
	cin >> keyValue;
	int keyIndex = 0;
	for (int i = 0; i < type.numberOfFields; i++) {
		if (type.keyField.compare(type.fieldNames[i]) == 0)
			keyIndex = i;
	}
	if (type.numberOfFields == NULL) {
		cout << "No such type name found" << endl;
		return -1;
	}
	vector<int> fieldValues;
	int numberOfRecordsInPage = 2039 / (sizeof(int) * type.numberOfFields + 1);
	dataFile.seekg(0, dataFile.end);
	int numberOfPages = (dataFile.tellg() / 2048);
	dataFile.seekg(0, dataFile.beg);
	bool isFull;
	bool isDeleted = true;
	bool found = false;
	int dummy = 0;
	bool dummybool;
	for (int t = 0; t < numberOfPages; t++) {
		isFull = false;
		dataFile.seekg(t * 2048 + 9, dataFile.beg);
		for (int i = 0; i < numberOfRecordsInPage; i++) {
			dataFile.read((char*)&record.isDeleted, sizeof(bool));
			for (int k = 0; k < type.numberOfFields; k++) {
				int x;
				dataFile.read((char*)&x, sizeof(int));
				fieldValues.push_back(x);
				//if index is key field index and the value matches the entered value
				//and the record is not deleted, it means we find the record to be deleted
				//change isDeleted field to true
				if (k == keyIndex && !record.isDeleted && keyValue == fieldValues.at(k)) {
					recordIndex = i;
					recordPage = t;
					found = true;
					dummybool = true;
					dataFile.seekg(t * 2048 + 9 + i *(1+ sizeof(int)* type.numberOfFields), dataFile.beg);
					dataFile.write((char*)&dummybool, sizeof(bool));
					cout << "Record deleted !!!";
					getchar(); getchar();
					dataFile.close();
					sysCat.close();
					return 1;
				}
			}
			

		}

	}
	cout << "couldn't find" << endl;
	return 0;
}
int updateRecord() {
	string typeName;
	int keyValue;
	int recordIndex = 0;
	int recordPage = 0;
	cout << "Enter the type of the record to be updated: ";
	cin >> typeName;

	fstream sysCat;
	sysCat.open("systemcatalogue.txt", ios::in | ios::out);
	fstream dataFile;
	dataFile.open(typeName + ".dat", ios::in | ios::out);
	RecordType type;
	//retrieve the type
	type = findType(sysCat, typeName);
	cout << "Enter the key value " + type.keyField + "  : ";
	cin >> keyValue;
	int keyIndex = 0;
	//find key filed index
	for (int i = 0; i < type.numberOfFields; i++) {
		if (type.keyField.compare(type.fieldNames[i]) == 0)
			keyIndex = i;
	}
	if (type.numberOfFields == NULL) {
		cout << "No such type name found" << endl;
		return -1;
	}
	//calculate number of records in a page
	int numberOfRecordsInPage = 2039 / (sizeof(int) * type.numberOfFields + 1);
	dataFile.seekg(0, dataFile.end);
	int numberOfPages = (dataFile.tellg() / 2048);
	dataFile.seekg(0, dataFile.beg);
	bool isFull;
	bool isDeleted = true;
	bool found = false;
	int dummy = 0;
	bool dummybool;
	for (int t = 0; t < numberOfPages; t++) {
		isFull = false;
		dataFile.seekg(t * 2048 + 9, dataFile.beg);
		for (int i = 0; i < numberOfRecordsInPage; i++) {
			vector<int> fieldValues;
			Record record;
			dataFile.read((char*)&record.isDeleted, sizeof(bool));
			for (int k = 0; k < type.numberOfFields; k++) {
				int x;
				dataFile.read((char*)&x, sizeof(int));
				fieldValues.push_back(x);
				//Check if twe found the record in database
				if (k == keyIndex && !record.isDeleted && keyValue == fieldValues.at(k)) {
					recordIndex = i;
					recordPage = t;
					found = true;
				}
			}
			//if found, ask for new value of that field and update it
			if (found) {
				int newValue;
				string fieldName;
				int index;
				bool deleted = 0;
				cout << "Enter the field name to be updated: ";
				cin >> fieldName;
				cout << "Enter new value for " + fieldName +"  :";
				cin >> newValue;
				for (int i = 0; i < type.numberOfFields; i++) {
					if (fieldName.compare(type.fieldNames[i]) == 0)
						index = i;
				}
				fieldValues[index] = newValue;
				dataFile.seekg(t * 2048 + 9 + i*(1+ sizeof(int) * type.numberOfFields), dataFile.beg);
				dataFile.write((char*)&deleted, sizeof(bool));
				for (int i = 0; i < type.numberOfFields; i++) {
					dataFile.write((char*)&fieldValues[i], sizeof(int));
				}
				cout << "Data updated !!!" << endl;
				getchar(); getchar();
				sysCat.close();
				dataFile.close();
				return 0;
			}

		}

	}
	cout << "couldn't find" << endl;
	return 0;
	return 0;
}
int searchRecord() {
	string typeName;
	int keyValue;
	int recordIndex = 0;
	int recordPage = 0;
	cout << "Enter the type of the record to be searched: ";
	cin >> typeName;

	fstream sysCat;
	sysCat.open("systemcatalogue.txt", ios::in | ios::out);
	fstream dataFile;
	dataFile.open(typeName + ".dat", ios::in | ios::out);
	RecordType type;
	
	type = findType(sysCat, typeName);
	cout << "Enter the key value  "+ type.keyField + " :";
	cin >> keyValue;
	int keyIndex = 0;
	for (int i = 0; i < type.numberOfFields; i++) {
		if (type.keyField.compare(type.fieldNames[i]) == 0)
			keyIndex = i;
	}
	if (type.numberOfFields == NULL) {
		cout << "No such type name found" << endl;
		return -1;
	}
	
	int numberOfRecordsInPage = 2039 / (sizeof(int) * type.numberOfFields + 1);
	dataFile.seekg(0, dataFile.end);
	int numberOfPages = (dataFile.tellg() / 2048);
	dataFile.seekg(0, dataFile.beg);
	bool isFull;
	bool isDeleted = true;
	bool found = false;
	int dummy = 0;
	bool dummybool;
	for (int t = 0; t < numberOfPages; t++) {
		isFull = false;
		dataFile.seekg(t * 2048 + 9, dataFile.beg);
		for (int i = 0; i < numberOfRecordsInPage; i++) {
			vector<int> fieldValues;
			Record record;
			dataFile.read((char*)&record.isDeleted, sizeof(bool));
			for (int k = 0; k < type.numberOfFields; k++) {
				int x;
				dataFile.read((char*)&x, sizeof(int));
				fieldValues.push_back(x);
				if (k == keyIndex && !record.isDeleted && keyValue == fieldValues[k]) {
					recordIndex = i;
					recordPage = t;
					found = true;
				}
			}
			if (found) {
				for (int k = 0; k < type.numberOfFields; k++) {
					cout << type.fieldNames[k] + "\t";
				}
				cout << ""<<endl;
				for (int k = 0; k < type.numberOfFields; k++) {
					cout <<	 fieldValues.at(k);
					cout << "\t";
				}
				cout << endl;
				cout << "Press a key to continue" << endl;
				getchar(); getchar();
				sysCat.close();
				dataFile.close();
				return 0;
			}

		}

	}
	cout << "couldn't find" << endl;
	return 0;
}
int listRecords() {
	string typeName;
	int keyValue;
	int recordIndex = 0;
	int recordPage = 0;
	cout << "Enter the type of the record to be listed: ";
	cin >> typeName;

	fstream sysCat;
	sysCat.open("systemcatalogue.txt", ios::in | ios::out);
	fstream dataFile;
	dataFile.open(typeName + ".dat", ios::in | ios::out);
	RecordType type;

	type = findType(sysCat, typeName);
	int keyIndex = 0;
	if (type.numberOfFields == NULL) {
		cout << "No such type name found" << endl;
		return -1;
	}
	
	int numberOfRecordsInPage = 2039 / (sizeof(int) * type.numberOfFields + 1);
	dataFile.seekg(0, dataFile.end);
	int numberOfPages = (dataFile.tellg() / 2048);
	dataFile.seekg(0, dataFile.beg);
	bool isFull;
	bool isDeleted = true;
	bool found = false;
	int dummy = 0;
	bool dummybool;
	for (int k = 0; k < type.numberOfFields; k++) {
		cout << type.fieldNames[k] + "\t";
	}
	cout << "" << endl;
	//goes to the file of that type and prints all records that are not deleted
	for (int t = 0; t < numberOfPages; t++) {
		isFull = false;
		dataFile.seekg(t * 2048 + 9, dataFile.beg);
		for (int i = 0; i < numberOfRecordsInPage; i++) {
			Record record;
			vector<int> fieldValues;
			dataFile.read((char*)&record.isDeleted, sizeof(bool));
			for (int k = 0; k < type.numberOfFields; k++) {
				int x;
				dataFile.read((char*)&x, sizeof(int));
				fieldValues.push_back(x);
				if ( !record.isDeleted ) {
					cout << fieldValues.at(k);
					cout << "\t";
				}
			}
			if(!record.isDeleted)
				cout << endl;
			

		}

	}
	return 0;
}

