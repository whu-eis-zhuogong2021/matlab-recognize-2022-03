close all
clear all
clc
%author:卓工一班：孙熠遥、张镒丞、宋翔

%读取音频文件

info =audioinfo('D:\dtmf-test1.wav');%获取音频文件的信息
[x,Fs]=audioread('D:\dtmf-test1.wav');     % 读入数据文件
if Fs==16000
    x1=x(:,2);
else
    x1=x;
end

wlen=200; inc=80;          % 给出帧长和帧移
win=hanning(wlen);         % 给出海宁窗
N=length(x1);               % 信号长度
X=enframe(x1,win,inc)';     % 分帧
fn=size(X,2);              % 求出帧数。size函数里选择1时是返回行数，2是返回列数
time=(0:N-1)/Fs;           % 计算出信号的时间刻度
for i=1 : fn
    u=X(:,i);              % 取出一帧
    u2=u.*u;               % 求出能量
    En(i)=sum(u2);         % 对一帧累加求和
end
subplot 211; plot(time,x,'k'); % 画出时间波形 
title('语音波形');
ylabel('幅值'); xlabel(['时间/s' 10 '(a)']);
frameTime=frame2time(fn,wlen,inc,Fs);   % 求出每帧对应的时间
subplot 212; plot(frameTime,En,'k')     % 画出短时能量图
title('短时能量');
ylabel('幅值'); xlabel(['时间/s' 10 '(b)']);
value=[];
j=1;
k=1;
while 1
for i1=k:3:fn
    
    if En(i1)>0.3
        value(j)=i1;
        j=j+1;
        k=i1;
        break;
    end
end
for i2=k:3:fn
    if En(i2)<0.3
        value(j)=i2;
        j=j+1;
        k=i2;
        break;
    end
end
if value(j-1)==value(j-2) 
    break;
end
end

xvalue=[];
for qwr=1:100
    t1=((value(qwr)-1)*inc+wlen/2)/Fs;
    xvalue(qwr)=floor(t1*Fs);
    if qwr~=1&&value(qwr)==value(qwr-1)
        break;
    end
end
%寻找端点时刻值对应的波形点数
%______________________________
high=[1209,1336,1477];
low=[697,770,852,941];
number=[1,2,3;4,5,6;7,8,9;0,0,0];
n=size(xvalue);
n1=floor(n(2)/2);
h=1;q=0;p=0;
fprintf('The number is: ');
while h<=n1
    Y_new=x(xvalue(h*2-1):xvalue(h*2));
    y=abs(fft(Y_new));       %做FFT变换
    f=(0:length(y)-1).*Fs/length(y);
    [pks,locs] = findpeaks(y,f,'MinPeakDistance',5);
    [a,b] = findpeaks(pks,locs,'minpeakheight',10);
    i=1;c=[];
    while b(i)<2000
        c(i)=b(i);
        i=i+1;
    end
    j=1;
    while j<i
        d=1;
        e=1;
        if c(j)<650
            j=j+1;
            continue
        end
    
        for d=1:4
            if (c(j)>low(d)-10)&&(c(j)<low(d)+10)
                l=low(d);
                q=d;
                break
            end
        end    
    
        for e=1:3
            if (c(j)>high(e)-10)&&(c(j)<high(e)+10)
                k=high(e);
                p=e;
                break
            end   
        end    
        j=j+1;
    end
% format = 'l = %f Hz;\n';
% fprintf(format,l);
% formatSpec='h = %f Hz;\n';
% fprintf(formatSpec,h);
if q==0||p==0
    fprintf(' ');
else    
    format1='%d';
    fprintf(format1,number(q,p));
end    
 h=h+1;q=0;p=0;
end

fprintf('\n');

function f=enframe(x,win,inc)
nx=length(x(:));            % 取数据长度
nwin=length(win);           % 取窗长
if (nwin == 1)              % 判断窗长是否为1，若为1，即表示没有设窗函数
   len = win;               % 是，帧长=win
else
   len = nwin;              % 否，帧长=窗长
end
if (nargin < 3)             % 如果只有两个参数，设帧inc=帧长
   inc = len;
end
nf = fix((nx-len+inc)/inc); % 计算帧数
f=zeros(nf,len);            % 初始化
indf= inc*(0:(nf-1)).';     % 设置每帧在x中的位移量位置
inds = (1:len);             % 每帧数据对应1:len
f(:) = x(indf(:,ones(1,len))+inds(ones(nf,1),:));   % 对数据分帧
if (nwin > 1)               % 若参数中包括窗函数，把每帧乘以窗函数
    w = win(:)';            % 把win转成行数据
    f = f .* w(ones(nf,1),:);  % 乘窗函数
end
end

function frameTime=frame2time(frameNum,framelen,inc,fs)
% 分帧后计算每帧对应的时间
frameTime=(((1:frameNum)-1)*inc+framelen/2)/fs;
end