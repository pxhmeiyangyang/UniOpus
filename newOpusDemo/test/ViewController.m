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
@property(nonatomic,strong)NSMutableData* data;
@property(nonatomic,assign)BOOL isEncode;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    _data = [NSMutableData data];
    _opus = [[libOpus alloc]init];
    _opus.delegate = self;
    _isEncode = true;
    NSString* type = @"pcm";
    if (_isEncode) {
        type = @"pcm";
    }else{
        type = @"opus";
    }
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"test" ofType:type];
    NSData* data = [NSData dataWithContentsOfFile:filePath];
    NSLog(@"%lu",(unsigned long)data.length);
    [_opus appendAudioData:data isEncode:_isEncode];
}


-(void)opusDataDidEncode:(NSData *)encodeData{
    [_data appendData:encodeData];
    NSLog(@"opusDataDidEncode:%@",encodeData);
}

/**
 *  编码完成回调
 */
-(void)opusDataDidFinished{
    NSLog(@"======编码完成了  : %ld",_data.length);
    NSString* cachePath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,  NSUserDomainMask, YES).firstObject;
    NSString* path = @"";
    if (_isEncode) {
        path = [NSString stringWithFormat:@"%@/cacheTest.opus",cachePath];
    }else{
        path = [NSString stringWithFormat:@"%@/cacheTest.pcm",cachePath];
    }
   
//    NSString* pcmPath = [[NSBundle mainBundle] pathForResource:@"testtest" ofType:@"opus"];
    if([_data writeToFile:path atomically:NO]){
        NSLog(@"写入成功");
    }
    
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
