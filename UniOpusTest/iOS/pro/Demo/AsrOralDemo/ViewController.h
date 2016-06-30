//
//  ViewController.h
//  AsrOralDemo
//
//  Created by 刘俊 on 15/1/7.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "EngineManager.h"

@interface ViewController : UIViewController<EngineManagerDelegate>
{
    NSMutableString *resultText;
    EngineManager *enginer;
}


@end

