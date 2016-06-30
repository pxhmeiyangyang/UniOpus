//
//  UniBackupInfo.h
//  usc
//
//  Created by 刘俊 on 15/12/2.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface UniBackupInfo : NSObject

@property (nonatomic, strong)NSNumber *isBackup;
@property (nonatomic, strong)NSDate *updateTime;

@end

//backupType 可迁移至本类进行统一管理
//将HTTP评测的备份状态设置移至USCRecognition的回调中做
//