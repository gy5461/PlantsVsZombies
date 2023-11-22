#pragma once

//ͷ�ļ�Ҫ��
#include <cmath>

struct vector2 {
	vector2(int _x=0, int _y=0) :x(_x), y(_y) {}
	vector2(int* data) :x(data[0]), y(data[1]){}
	long long x, y;
};

//�ӷ�
vector2 operator +(vector2 x, vector2 y);

//����
vector2 operator -(vector2 x, vector2 y);

// �˷�
vector2 operator *(vector2 x, vector2 y);
vector2 operator *(vector2, float);
vector2 operator *(float, vector2);

//���
long long cross(vector2 x, vector2 y);

//������ ���
long long dot(vector2 x, vector2 y);

//�����������
long long dv(long long a, long long b);


//ģ��ƽ��
long long len(vector2 x);

//ģ��
long long dis(vector2 x);

//��������
vector2 operator /(vector2 x, vector2 y);

//����Ĥ
vector2 operator %(vector2 x, vector2 y);

//����GCD 
vector2 gcd(vector2 x, vector2 y);

// 贝塞尔曲线
vector2 calcBezierPoint(float t, vector2 p0, vector2 p1, vector2 p2, vector2 p3);