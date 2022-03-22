
function [f] = fra(len,inc,x)
%fra 对语音信号分帧
%   len-帧长，inc-非重叠样点长度，x-语音信号
fh=fix((size(x,1)-len+inc)/inc);  %计算帧数
f=zeros(fh,len);                  %行为帧长，列为帧数
i=1;n=1;
while i<fh                        %帧间循环
    j=1;
    while j<len                   %帧内循环
        f(i,j)=x(n);
        j=j+1;
        n=n+1;
    end
    n=n-len+inc;                  %下一帧开始的位置
    i=i+1;
end
end