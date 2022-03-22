
clear
clc

[x,Fs]=audioread('dtmf-test2.wav');
%sound(x0,Fs);
x0=x(:,1);                                 %取左声道
N=length(x0);                              %采样点
k=(0:N-1);
f=(k/N-1/2)*Fs;
X0=fft(x0);
figure(1);
subplot(2,1,1),plot(x0)
title('原始按键音(时域）'),xlabel('t'),ylabel('振幅')
subplot(2,1,2),plot(f,abs(fftshift(X0)));xlim([0,4000])
title('原始按键音（频域）'),xlabel('f/Hz'),ylabel('幅度')                                 

% -----------------------------------------带通滤波 
Hd=band_pass(Fs);                          %带通滤波器
x1=filter(Hd,x0);                          %滤波
%sound(x1,Fs)
X1=fft(x1);
figure(2);
subplot(2,1,1),plot(x1)
title('滤波后的按键音(时域）'),xlabel('t'),ylabel('振幅')%画出傅里叶变换后的单侧频谱
subplot(2,1,2),plot(f,abs(fftshift(X1))),xlim([0,2000])
title('滤波后的按键音（频域）'),xlabel('f/Hz'),ylabel('幅度') 

% -------------------------------------------短时能量
len=256;                                    %帧长
d=100;                                      %帧重叠样点长
[s]=fra(len,len-d,x1);                      %分帧,s为帧数
es=s.^2;                                    %一帧内各样点能量
energy=sum(es,2);                           %一帧的能量,行求和                  
figure(3);
subplot(2,1,1),plot(x1)
title('按键音'),ylabel('幅度')
subplot(2,1,2),plot(energy)
title('短时能量'),xlabel('帧编号'),ylabel('E')

%-------------------------------------------端点检测
flag=energy;                                %有效信号标志
Ethresh=0.02;                               %短时能量阈值
flag(find(energy>Ethresh))=1;               %每一帧能量大于0.02的为有效值
flag(find(energy<=Ethresh))=0;                         
desired_signal=[];                          %有效信号标志
desired_signal(1)=0;                       
for i=1:length(flag)
    for j=2:i
        if flag(j-1)*flag(j)==0             %当相邻的两帧值均大于短时能量阈值时，其为有效信号
            desired_signal(i)=0;
        else
            desired_signal(i)=1;
        end
    end
end 
figure(4);plot(desired_signal),ylim([0,1.2])
title('有效信号标志（0无效，1有效）'),xlabel('帧编号'),ylabel('y')
for i=2:length(desired_signal)               %有效信号长度
    if desired_signal(i)-desired_signal(i-1)==1
        left(i)=i;                           %左端点
    elseif desired_signal(i)-desired_signal(i-1)==-1
        right(i)=i;                          %右端点
    end
end
left_end=find(left~=0);                      %左端点  left不等于0的位置即有效信号的帧数编号
right_end=find(right~=0);                    %右端点


%---------------------------------------分帧后的恢复，分割信号
[leftend1,leftend2]=inverse_fra(left_end,len-d,len);%将帧数编号还原为初始的帧点数，左边有两条线为出现有效信号的第一帧
[rightend1,rightend2]=inverse_fra(right_end,len-d,len);
figure(5);
subplot(2,1,1),plot(x1)
title('按键音'),ylabel('幅度'),xlabel('t')
for i=1:length(leftend1)
    line([leftend1(i) leftend1(i)],[-0.1 0.1],'Color','red')%在leftend1处画一条长度为0.2的线
    line([rightend1(i) rightend1(i)],[-0.1 0.1],'Linestyle','- -','Color','red')
end
subplot(2,1,2),plot(energy),ylim([-0.1,0.6])
title('短时能量'),xlabel('帧编号'),ylabel('E')
for i=1:length(left_end)
    line([left_end(i) left_end(i)],[-0.1 1],'Color','red')%在短时能量图中画出其对应帧编号的分割
    line([right_end(i) right_end(i)],[-0.1 1],'Linestyle','- -','Color','red')
end

%---------------------------------------取出音频数据，得到其双频率
fs_result = [];
for i = 1:length(leftend1)
   temp = x1(leftend1(i):rightend1(i));                    %取一段音频的数据
    temp = fft(temp);                                      %对音频进行傅里叶变换后得到的数据
    P2 = abs(temp/length(temp));                           %计算每个信号的双侧频谱和单侧频谱
    P1 = P2(1:length(temp)/2+1);
    P1(2:end-1) = 2*P1(2:end-1);                            %p1为单侧频谱
    f = Fs*(0:(length(temp)/2))/length(temp);               %f为频域
    
    [pk1,lc1] = findpeaks(P1,'SortStr','descend','NPeaks',2);%指定峰值排序方向为下降排列，并且寻找到2个峰值
    fs_result = [fs_result;f(lc1)];                          %不断将新得到的峰值处的频率存入结果中
end


%---------------------------------------对fs_result进行排序，运算找到最接近的频率
bohao = [697,1209;697,1336;697,1477;770,1209;770,1336;770,1477;852 1209;852 1336;852 1477;941 1336;941 1209;941 1477];%从1-9-0-*-#的频率

for i = 1:length(fs_result)
   fs_result(i,:) = sort(fs_result(i,:));%使得低频在前，高频在后
end
err = [];
number = [];
for i =1:length(fs_result)
    for j = 1:length(bohao)
        err = [err (fs_result(i,1)-bohao(j,1))^2+(fs_result(i,2)-bohao(j,2))^2];%存储实际数据与每一个拨号音的差值
    end
    
    if min(err) < 40
        temp = find(err == min(err));%返回与拨号音频率差值最小的值的位置即是电话号码的数字min（err）的位置
        if temp == 10
            temp = '*';
        end
        if temp == 11
            temp = '0';
        end
        if temp == 12
            temp = '#';
        end
        number = [number num2str(temp)];%不断存储得到的新数据
        err = [];
    end
end
disp('输入的电话号码为：');
disp(number);
