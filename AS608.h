#ifndef __AS608_H__
#define __AS608_H__

//指纹模块初始化
void F_Model_Init();
//在现有指纹库检索匹配指纹
void FPM10A_Find_Fingerprint();	
//添加新的指纹
void FPM10A_Add_Fingerprint();	
//删除全部的指纹
void FPM10A_Delete_All_Fingerprint();

#endif
