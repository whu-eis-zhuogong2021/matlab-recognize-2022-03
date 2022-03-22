
function [end1,end2] = inverse_fra(k,inc,len)
%inverse_fra 将帧数编号时，还原到原始语音部分
%输出   end1-起始端点，end2-结束端点
%输入   k-帧编号，inc-帧非重叠样点长度，len-帧长
end1=(k-1)*inc+1;
end2=(k-1)*inc+len;
end