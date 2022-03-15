close all
clear all
clc
 
%读取音频文件
info =audioinfo('D:\dtmf-test1.wav');%获取音频文件的信息
[audio,Fs] = audioread('D:\dtmf-test1.wav');%读取音频文件
sound(audio,Fs);%播放音频文件
audiolength = length(audio);%获取音频文件的数据长度
t = 1:1:audiolength;

figure(1),
plot(t,audio(1:audiolength));
xlabel('Time');
ylabel('Audio Signal');
title('原始音频文件信号幅度图');

Y_new=audio(33346:35032);

y=fft(Y_new);       %做FFT变换
f=(0:length(y)-1).*Fs/length(y);
plot(f,y);
%x = audioinfo('D:\dtmf-test2.wav');

[x,Fs]=audioread('D:\dtmf-test1.wav');     % 读入数据文件
wlen=200; inc=80;          % 给出帧长和帧移
win=hanning(wlen);         % 给出海宁窗
N=length(x);               % 信号长度
X=enframe(x,win,inc)';     % 分帧
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

