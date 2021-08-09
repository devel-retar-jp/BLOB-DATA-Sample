#include <Windows.h>
#include <Bcrypt.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "functions.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// ��������
/// </summary>
/// <param name="min">�ŏ�</param>
/// <param name="max">�ő�</param>
int randnumber(int min, int max)
{
	std::random_device rd;									//��������
	std::mt19937_64 mt(rd());								//�����Z���k�E�c�C�X�^�[�@�ɂ���Đ�������闐����
	std::uniform_int_distribution<int> uid(min, max);		//�ő�ŏ��̐���
	return uid(mt);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// �X�e�[�^�X�`�F�b�N
/// </summary>
/// <param name="status">�֐����^�[���X�e�[�^�X</param>
/// <param name="Message">���b�Z�[�W/param>
void statusCheck(NTSTATUS	status, std::wstring Message)
{
	if (NT_SUCCESS(status)) {
		std::wcout << Message << std::endl;
	}
	else {
		std::wcout << "*** Error 0x" << std::hex << status << " returned by " << Message << std::endl;
		throw std::exception();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// �t�@�C�������o��
/// </summary>
/// <param name="outstring">�����o���f�[�^�|�C���^</param>
/// <param name="outlength">�����o���f�[�^��</param>
/// <param name="FileName">�t�@�C����</param>
/// <param name="type">�����o���X�^�C���E�i16/10/0�j�E16�i���A10�i���A0�o�C�i��</param>
void writeFileData(LPBYTE outstring, unsigned long outlength, std::wstring* FileName, int type)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TCHAR dir[MAX_PATH] = { 0 };												//�p�X�̍ő�l�̔z���p��
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GetModuleFileName(NULL, dir, MAX_PATH);										//���s�t�@�C�������擾
	//std::wcout << "Current Dir ROW   : " << dir << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::wstring::size_type pos = std::wstring(dir).find_last_of(DirSep);		//�Ō��"\"�̈ʒu��pos�Ɏ擾
	std::wstring currentDirString = std::wstring(dir).substr(0, pos);			//currentDirString�ɕ������擾
	std::wstring currentDirStringSV = currentDirString;							//�ۑ�
	//std::wcout << "Current Dir String: " << currentDirString << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	currentDirString = currentDirStringSV;										//������A��
	currentDirString += DirSep;													//������A��
	currentDirString += *FileName;												//������A��
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//�����o���E�X�g���[���Ȃ̂ŁA�}���}���������OK
	std::basic_ofstream<BYTE> ofsP(currentDirString, std::ios::binary);			//�t�@�C���I�[�v��
	for (unsigned long i = 0; i < outlength; i++)
	{
		std::stringstream decStr, hexStr;
		std::string s;

		switch (type)
		{
			//10�i�������o��
		case 10:
			decStr << std::setfill('0') << std::setw(3) << std::dec << (unsigned long)outstring[i] << " ";
			s = decStr.str();
			ofsP << (BYTE)s[0];
			ofsP << (BYTE)s[1];
			ofsP << (BYTE)s[2];
			if (i != outlength - 1)
			{
				if ((i + 1) % type == 0 && i != 0)
				{
					ofsP << "\n";
				}
				else
				{
					ofsP << (BYTE)s[3];
				}
			}
			break;
			//16�i�������o��
		case 16:
			hexStr << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned long)outstring[i] << " ";
			s = hexStr.str();
			ofsP << (BYTE)s[0];
			ofsP << (BYTE)s[1];
			if (i != outlength - 1)
			{
				if ((i + 1) % type == 0 && i != 0)
				{
					ofsP << "\n";
				}
				else
				{
					ofsP << (BYTE)s[2];
				}
			}
			break;
			//�o�C�i�������o��
		default:
			ofsP << outstring[i];
		}
	}
	ofsP.close();																//�̂̃R���p�C���Ȃ�N���[�Y�Y��̓o�O�̌��ł����A�����Ă����Ȃ�
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// �t�@�C���ǂݍ��݁E�ۂ���
/// </summary>
/// <param name="pcurrentDirString">�t���p�X�t�@�C����</param>
/// <returns>�ǂݍ��݃f�[�^</returns>
std::vector<BYTE> readFileData(LPWSTR pcurrentDirString)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// std::vector<BYTE>�ɓǂݍ��ށE�C�L�i���ǂݍ��݁E����2
	std::basic_ifstream<BYTE> file2(pcurrentDirString, std::ios::binary);		//�t�@�C���I�[�v��
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// basic_ifstream�̃C�e���[�^������̂œǂ�ł��
	return std::vector<BYTE>((std::istreambuf_iterator<BYTE>(file2)), std::istreambuf_iterator<BYTE>());
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// �t�@�C���E�p�X�擾
/// </summary>
/// <param name="FileName">�t�@�C����</param>
/// <returns>�t���p�X�t�@�C����</returns>
std::wstring makeFilePath(std::wstring* FileName)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TCHAR dir[MAX_PATH] = { 0 };												//�p�X�̍ő�l�̔z���p��
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GetModuleFileName(NULL, dir, MAX_PATH);										//���s�t�@�C�������擾
	//std::wcout << "Current Dir ROW   : " << dir << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::wstring::size_type pos = std::wstring(dir).find_last_of(DirSep);		//�Ō��"\"�̈ʒu��pos�Ɏ擾
	std::wstring currentDirString = std::wstring(dir).substr(0, pos);			//currentDirString�ɕ������擾
	std::wstring currentDirStringSV = currentDirString;							//�ۑ�
	//std::wcout << "Current Dir String: " << currentDirString << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	currentDirString += DirSep;													//������A��
	currentDirString += *FileName;												//������A��
	//LPWSTR pcurrentDirString = &currentDirString[0];							//std::wstring -> LPWSTR
	//std::wcout << "Current Dir CONN  : " << pcurrentDirString << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	return currentDirString;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// �o�͊֐�
/// </summary>
/// <param name="outstring">LPBYTE�^�̕�����</param>
/// <param name="outlength">������̒���</param>
/// <param name="banner">�o�i�[</param>
void binStdOut(LPBYTE outstring, unsigned long outlength, std::string banner)
{	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Banner �o��
	std::cout << "----------------------------------------------------------" << "\n";
	std::cout << banner << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//�f�[�^�o��
	unsigned long datapoint = 0;
	unsigned long countdiv = 16;
	unsigned long i = 0;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::cout << "         | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F" << "\n";
	std::cout << "----------------------------------------------------------" << "\n";
	//for (unsigned long i = 0; i < countdiv; i++)
	while (1)
	{
		std::cout << std::setfill('0') << std::setw(8) << std::uppercase << std::hex << i * countdiv << " | ";
		for (unsigned long j = 0; j < countdiv; j++)
		{
			auto s = outstring[datapoint];
			//16
			std::cout << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned long)s << " ";
			//std::cout << std::setfill('0') << std::setw(3) << std::uppercase << std::dec << (unsigned long)outstring[datapoint] << " ";
			datapoint++;
			if (outlength <= datapoint) { break; }
		}
		std::cout << "\n";
		if (outlength <= datapoint) { break; }
		i++;
	}
	std::cout << "----------------------------------------------------------" << "\n";
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
