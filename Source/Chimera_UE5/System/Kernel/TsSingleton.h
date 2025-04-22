// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

template<typename T>
class TsSingleton
{
protected:
	static T* gInstance;
public:
	static T* Instance() { return gInstance; }

	TsSingleton() = default;// �f�t�H���g�R���X�g���N�^��ǉ�
	TsSingleton(T* inst) { gInstance = inst; }
	~TsSingleton() { gInstance = nullptr; }
};

template<typename T>
T* TsSingleton<T>::gInstance = nullptr;


//�@ ���t���N�V�����E�K�x�[�W�R���N�V�����Ƃ̔�݊���
//	- UE�� UObject �x�[�X�̃I�u�W�F�N�g�����t���N�V�����V�X�e���ŊǗ����Ă���
//	- UObject �̓K�x�[�W�R���N�V�����iGC�j�̑ΏۂɂȂ�
//	- �ʏ��Singleton�� UObject �̃C���X�^���X�Ǘ���j�󂵂Ă��܂��\��������
// 
//	?? UE�̊Ǘ��O�ɂ���Singleton�C���X�^���X �� �K�x�[�W�R���N�V�����ŏ������ �� �N���b�V������
//

//�A �z�b�g�����[�h��G�f�B�^�ċN�����̕s�
//	- UE�́u�z�b�g�����[�h�v��uPIE�iPlay In Editor�j�v���ɃI�u�W�F�N�g���Đ��������
//	- Singleton�C���X�^���X���X�^�e�B�b�N�ȏꍇ�A
//		�� �����[�h���ɏ��������ꂸ�A�s���ȃC���X�^���X�Q�Ƃ��c��\��
// 
//	?? �����[�h���̕s���肳 �� �G�f�B�^���N���b�V�����鋰��
//

//�B �G�f�B�^�̃��t���N�V�����E�v���p�e�B�ҏW�������������
//	- UCLASS �� UObject �x�[�X�ō쐬����邽�߁A
//		Singleton�ɂ����ꍇ UCLASS �̕ҏW��C���X�y�N�V�������ł��Ȃ��Ȃ邱�Ƃ�����
// 
//?? Singleton���Ń��t���N�V������������ �� �f�o�b�O�E�v���p�e�B�ݒ肪����ɂȂ�
//
