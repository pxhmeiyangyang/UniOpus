//
//  ViewController.m
//  test
//
//  Created by pxh on 2016/12/9.
//  Copyright © 2016年 pxh. All rights reserved.
//

#import "ViewController.h"

@interface ViewController()
@property(nonatomic,strong)libOpus* opus;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    _opus = [[libOpus alloc]init];
    _opus.delegate = self;
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"test" ofType:@"pcm"];
    NSData* data = [NSData dataWithContentsOfFile:filePath];
    NSLog(@"%lu",(unsigned long)data.length);
    [_opus appendAudioData:data];
}


-(void)opusDataDidEncode:(NSData *)encodeData{
}

/**
 *  编码完成回调
 */
-(void)opusDataDidFinished{
    NSLog(@"======编码完成了");
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
