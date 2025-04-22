// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

template<typename T>
class TsSingleton
{
protected:
	static T* gInstance;
public:
	static T* Instance() { return gInstance; }

	TsSingleton() = default;// デフォルトコンストラクタを追加
	TsSingleton(T* inst) { gInstance = inst; }
	~TsSingleton() { gInstance = nullptr; }
};

template<typename T>
T* TsSingleton<T>::gInstance = nullptr;


//① リフレクション・ガベージコレクションとの非互換性
//	- UEは UObject ベースのオブジェクトをリフレクションシステムで管理している
//	- UObject はガベージコレクション（GC）の対象になる
//	- 通常のSingletonは UObject のインスタンス管理を破壊してしまう可能性がある
// 
//	?? UEの管理外にあるSingletonインスタンス → ガベージコレクションで消される → クラッシュ発生
//

//② ホットリロードやエディタ再起動時の不具合
//	- UEは「ホットリロード」や「PIE（Play In Editor）」時にオブジェクトが再生成される
//	- Singletonインスタンスがスタティックな場合、
//		→ リロード時に初期化されず、不正なインスタンス参照が残る可能性
// 
//	?? リロード時の不安定さ → エディタがクラッシュする恐れ
//

//③ エディタのリフレクション・プロパティ編集が無効化される
//	- UCLASS は UObject ベースで作成されるため、
//		Singletonにした場合 UCLASS の編集やインスペクションができなくなることがある
// 
//?? Singleton化でリフレクションが無効化 → デバッグ・プロパティ設定が困難になる
//
