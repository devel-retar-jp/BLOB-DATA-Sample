#pragma once
//#include <Windows.h>
//#include <Bcrypt.h>
//#include <iostream>
//#include <vector>
//#include <iterator>
//#include <string>
//#include <fstream>
//#include <sstream>
//#include <iomanip>
//#include <random>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DirSep  L"\\"															//�f�B���N�g���̋�؂蕶����
#define	NT_SUCCESS(Status)		(((NTSTATUS)(Status)) >= 0)
#define	STATUS_UNSUCCESSFULL	((NTSTATUS)0xC0000001L)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// �t�@�C���E�p�X�擾
std::wstring makeFilePath(std::wstring* FileName);
/// �o�͊֐�
void binStdOut(LPBYTE outstring, unsigned long outlength, std::string banner);
/// �t�@�C���ǂݍ���
std::vector<BYTE> readFileData(LPWSTR pcurrentDirString);
/// �t�@�C�������o��
void writeFileData(LPBYTE outstring, unsigned long outlength, std::wstring* FileName, int type);
/// �X�e�[�^�X�`�F�b�N
void statusCheck(NTSTATUS	status, std::wstring Message);
/// ��������
int randnumber(int min, int max);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
