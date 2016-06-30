//
//  SettingController.m
//  AsrOralDemo
//
//  Created by 刘俊 on 15/6/25.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

/*
 zzy 1212
 xyz 1019
 zw 2546
 */

#define SettingCell @"setting"

#import "SettingController.h"
#import "EngineManager.h"

@interface SettingController ()<UITextFieldDelegate>
{
    EngineManager *enginer;
    
    //---------------------
    NSMutableDictionary *recognizeModeDic;
    
    IBOutlet UITextField *scoreTie;
    __weak IBOutlet UITextField *offlineWaitTime;
}

@end

@implementation SettingController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    enginer = [EngineManager sharedManager];
    
    [self initRecognizeMode];
    
    scoreTie.delegate  = self;
    
    offlineWaitTime.delegate = self;
    
    self.tableView.tableHeaderView = [[UIView alloc]init];
    
    [self.tableView registerClass:[UITableViewCell class] forCellReuseIdentifier:SettingCell];
}

-(void)finish
{
    NSLog(@"BBB");
}

-(void)initRecognizeMode
{
    recognizeModeDic = [[NSMutableDictionary alloc]init];
    [recognizeModeDic setObject:@"0" forKey:@"A"];
    [recognizeModeDic setObject:@"0" forKey:@"B"];
    [recognizeModeDic setObject:@"0" forKey:@"C"];
    [recognizeModeDic setObject:@"0" forKey:@"D"];
    [recognizeModeDic setObject:@"0" forKey:@"E"];
    [recognizeModeDic setObject:@"0" forKey:@"enstar"];
    [recognizeModeDic setObject:@"0" forKey:@"gzedunet"];
    [recognizeModeDic setObject:@"0" forKey:@"gzedunet_answer"];
}

-(void)viewWillAppear:(BOOL)animated
{
    //获取引擎当前评测模式
    NSString *oralTask = [enginer getOralTask];
    //[recognizeModeDic setObject:@"1" forKey:oralTask];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

-(NSString *)getRecordingMode
{
    RecognizeMode mode = [enginer getRecognizerMode];
    if (mode == Recognize_REC)
    {
        return @"麦克风";
    }
    else if (mode == Recognize_PCM)
    {
        return @"PCM";
    }
    else
    {
        return NULL;
    }
}

-(void)changeRecordingMode
{
    RecognizeMode mode = [enginer getRecognizerMode];
    if (mode == Recognize_REC)
    {
        [enginer setRecognizerMode:Recognize_PCM];
    }
    else if (mode == Recognize_PCM)
    {
        [enginer setRecognizerMode:Recognize_REC];
    }
}

-(void)changeRecognizeMode:(NSString *)modeStr
{
    NSArray *allKeys = [recognizeModeDic allKeys];
    for (int i = 0; i < allKeys.count; i++)
    {
        NSString *key = allKeys[i];
        if ([key isEqualToString:modeStr])
        {
            [recognizeModeDic setObject:@"1" forKey:key];
        }
        else
        {
            [recognizeModeDic setObject:@"0" forKey:key];
        }
    }
    
    [enginer setOralTask:modeStr];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 4;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    NSInteger number = 0;
    if (section == 0)
    {
        number = 2;
    }
    else if (section == 1)
    {
        number = recognizeModeDic.count;
    }else if(section == 2){
        number = 1;
    }else if (section == 3){
        number = 1;
    }
    return number;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:SettingCell];
    if(cell != NULL)
    {
        if (indexPath.section == 0)
        {
            if (indexPath.row == 0)
            {
                cell.textLabel.text = @"重置引擎状态";
            }
            else if (indexPath.row == 1)
            {
                NSString *modeStr= [NSString stringWithFormat:@"录音模式 : %@",[self getRecordingMode]];
                cell.textLabel.text = modeStr;
            }
        }
        else if(indexPath.section == 1)
        {
            NSArray *allKeys = [recognizeModeDic allKeys];
            if (allKeys != NULL)
            {
                cell.textLabel.text = allKeys[indexPath.row];
                NSString *value = recognizeModeDic[cell.textLabel.text];
                if ([value isEqualToString:@"1"])
                {
                    [cell setAccessoryType:UITableViewCellAccessoryCheckmark];
                }
                else
                {
                    [cell setAccessoryType:UITableViewCellAccessoryNone];
                }
            }
        }else if(indexPath.section == 2){
            NSString* str = @"";
            if ([enginer getAsyncRecognize]) {
                str = @"开启";
            }else{
                str = @"关闭";
            }
            cell.textLabel.text = [NSString stringWithFormat:@"延时评测：%@",str];
        }
        else if(indexPath.section == 3){
            NSString* str = @"";
            if (enginer.isOnlineWhenMix) {
                str = @"开启";
            }else{
                str = @"关闭";
            }
            cell.textLabel.text = [NSString stringWithFormat:@"混合版本只使用离线：%@",str];
        }
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    
    if (indexPath.section == 0)
    {
        if (indexPath.row == 1)
        {
            [self changeRecordingMode];
            
            NSString *modeStr= [NSString stringWithFormat:@"录音模式 : %@",[self getRecordingMode]];
            cell.textLabel.text = modeStr;
        }
    }
    else if (indexPath.section == 1)
    {
        NSString *modeStr = cell.textLabel.text;
        [self changeRecognizeMode:modeStr];
        NSString *value = recognizeModeDic[modeStr];
        if ([value isEqualToString:@"1"])
        {
            [cell setAccessoryType:UITableViewCellAccessoryCheckmark];
        }
        else
        {
            [cell setAccessoryType:UITableViewCellAccessoryNone];
        }
        
        [self.tableView reloadData];
    }else if(indexPath.section == 2){
        BOOL asyncRecognize = [enginer getAsyncRecognize];
        [enginer setAsyncRecognize:!asyncRecognize];
        [self.tableView reloadData];
    }else if(indexPath.section == 3){
        BOOL isOnlineWhenMix = enginer.isOnlineWhenMix;
        enginer.isOnlineWhenMix = !isOnlineWhenMix;
        [self.tableView reloadData];
    }
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    if (section == 0)
    {
        return [enginer version];
    }
    else if (section == 1)
    {
        return @"识别模式";
    }else if(section == 2){
        return @"延时评测";
    }else if(section == 3){
        return @"混合版本离在线选择";
    }
    
    return NULL;
}

#pragma mark - UITextFieldDelegate
- (BOOL)textFieldShouldEndEditing:(UITextField *)textField{
    if (textField.tag == 2002) {
        float time = [textField.text floatValue];
        [enginer setOfflineResultWaitingTime:time];
    }
    return YES;
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
