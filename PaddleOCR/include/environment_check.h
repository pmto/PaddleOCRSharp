////////////////////////////////////////////////////////////
// checkavx.cpp : 检测AVX系列指令集的支持级别.
// Author: zyl910
// Blog: http://www.cnblogs.com/zyl910
// URL: http://www.cnblogs.com/zyl910/archive/2012/07/04/checkavx.html
// Version: V1.0
// Updata: 2012-07-04
////////////////////////////////////////////////////////////
#ifdef _WIN64

namespace Environment
{
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
 
#if _MSC_VER >=1400	// VC2005才支持intrin.h
#include <intrin.h>	// 所有Intrinsics函数
#else
#include <emmintrin.h>	// MMX, SSE, SSE2
#endif


	// CPUIDFIELD
	typedef INT32 CPUIDFIELD;

#define  CPUIDFIELD_MASK_POS	0x0000001F	// 位偏移. 0~31.
#define  CPUIDFIELD_MASK_LEN	0x000003E0	// 位长. 1~32
#define  CPUIDFIELD_MASK_REG	0x00000C00	// 寄存器. 0=EAX, 1=EBX, 2=ECX, 3=EDX.
#define  CPUIDFIELD_MASK_FIDSUB	0x000FF000	// 子功能号(低8位).
#define  CPUIDFIELD_MASK_FID	0xFFF00000	// 功能号(最高4位 和 低8位).

#define CPUIDFIELD_SHIFT_POS	0
#define CPUIDFIELD_SHIFT_LEN	5
#define CPUIDFIELD_SHIFT_REG	10
#define CPUIDFIELD_SHIFT_FIDSUB	12
#define CPUIDFIELD_SHIFT_FID	20

#define CPUIDFIELD_MAKE(fid,fidsub,reg,pos,len)	(((fid)&0xF0000000) \
	| ((fid)<<CPUIDFIELD_SHIFT_FID & 0x0FF00000) \
	| ((fidsub)<<CPUIDFIELD_SHIFT_FIDSUB & CPUIDFIELD_MASK_FIDSUB) \
	| ((reg)<<CPUIDFIELD_SHIFT_REG & CPUIDFIELD_MASK_REG) \
	| ((pos)<<CPUIDFIELD_SHIFT_POS & CPUIDFIELD_MASK_POS) \
	| (((len)-1)<<CPUIDFIELD_SHIFT_LEN & CPUIDFIELD_MASK_LEN) \
	)
#define CPUIDFIELD_FID(cpuidfield)	( ((cpuidfield)&0xF0000000) | (((cpuidfield) & 0x0FF00000)>>CPUIDFIELD_SHIFT_FID) )
#define CPUIDFIELD_FIDSUB(cpuidfield)	( ((cpuidfield) & CPUIDFIELD_MASK_FIDSUB)>>CPUIDFIELD_SHIFT_FIDSUB )
#define CPUIDFIELD_REG(cpuidfield)	( ((cpuidfield) & CPUIDFIELD_MASK_REG)>>CPUIDFIELD_SHIFT_REG )
#define CPUIDFIELD_POS(cpuidfield)	( ((cpuidfield) & CPUIDFIELD_MASK_POS)>>CPUIDFIELD_SHIFT_POS )
#define CPUIDFIELD_LEN(cpuidfield)	( (((cpuidfield) & CPUIDFIELD_MASK_LEN)>>CPUIDFIELD_SHIFT_LEN) + 1 )

	// 取得位域
#ifndef __GETBITS32
#define __GETBITS32(src,pos,len)	( ((src)>>(pos)) & (((UINT32)-1)>>(32-len)) )
#endif


#define CPUF_SSE4A	CPUIDFIELD_MAKE(0x80000001,0,2,6,1)
#define CPUF_AES	CPUIDFIELD_MAKE(1,0,2,25,1)
#define CPUF_PCLMULQDQ	CPUIDFIELD_MAKE(1,0,2,1,1)

#define CPUF_AVX	CPUIDFIELD_MAKE(1,0,2,28,1)
#define CPUF_AVX2	CPUIDFIELD_MAKE(7,0,1,5,1)
#define CPUF_OSXSAVE	CPUIDFIELD_MAKE(1,0,2,27,1)
#define CPUF_XFeatureSupportedMaskLo	CPUIDFIELD_MAKE(0xD,0,0,0,32)
#define CPUF_F16C	CPUIDFIELD_MAKE(1,0,2,29,1)
#define CPUF_FMA	CPUIDFIELD_MAKE(1,0,2,12,1)
#define CPUF_FMA4	CPUIDFIELD_MAKE(0x80000001,0,2,16,1)
#define CPUF_XOP	CPUIDFIELD_MAKE(0x80000001,0,2,11,1)


// SSE系列指令集的支持级别. simd_sse_level 函数的返回值。
#define SIMD_SSE_NONE	0	// 不支持
#define SIMD_SSE_1	1	// SSE
#define SIMD_SSE_2	2	// SSE2
#define SIMD_SSE_3	3	// SSE3
#define SIMD_SSE_3S	4	// SSSE3
#define SIMD_SSE_41	5	// SSE4.1
#define SIMD_SSE_42	6	// SSE4.2

	const char* simd_sse_names[] = {
		"None",
		"SSE",
		"SSE2",
		"SSE3",
		"SSSE3",
		"SSE4.1",
		"SSE4.2",
	};


	// AVX系列指令集的支持级别. simd_avx_level 函数的返回值。
#define SIMD_AVX_NONE	0	// 不支持
#define SIMD_AVX_1	1	// AVX
#define SIMD_AVX_2	2	// AVX2

	const char* simd_avx_names[] = {
		"None",
		"AVX",
		"AVX2"
	};

	char szBuf[64];
	INT32 dwBuf[4];

#if defined(_WIN64)
	// 64位下不支持内联汇编. 应使用__cpuid、__cpuidex等Intrinsics函数。
#else
#if _MSC_VER < 1600	// VS2010. 据说VC2008 SP1之后才支持__cpuidex
	void __cpuidex(INT32 CPUInfo[4], INT32 InfoType, INT32 ECXValue)
	{
		if (NULL == CPUInfo)	return;
		_asm {
			// load. 读取参数到寄存器
			mov edi, CPUInfo;	// 准备用edi寻址CPUInfo
			mov eax, InfoType;
			mov ecx, ECXValue;
			// CPUID
			cpuid;
			// save. 将寄存器保存到CPUInfo
			mov[edi], eax;
			mov[edi + 4], ebx;
			mov[edi + 8], ecx;
			mov[edi + 12], edx;
		}
	}
#endif	// #if _MSC_VER < 1600	// VS2010. 据说VC2008 SP1之后才支持__cpuidex

#if _MSC_VER < 1400	// VC2005才支持__cpuid
	void __cpuid(INT32 CPUInfo[4], INT32 InfoType)
	{
		__cpuidex(CPUInfo, InfoType, 0);
	}
#endif	// #if _MSC_VER < 1400	// VC2005才支持__cpuid

#endif	// #if defined(_WIN64)

 
	// 根据CPUIDFIELD从缓冲区中获取字段.
	UINT32	getcpuidfield_buf(const INT32 dwBuf[4], CPUIDFIELD cpuf)
	{
		return __GETBITS32(dwBuf[CPUIDFIELD_REG(cpuf)], CPUIDFIELD_POS(cpuf), CPUIDFIELD_LEN(cpuf));
	}

	// 根据CPUIDFIELD获取CPUID字段.
	UINT32	getcpuidfield(CPUIDFIELD cpuf)
	{
		INT32 dwBuf[4];
		__cpuidex(dwBuf, CPUIDFIELD_FID(cpuf), CPUIDFIELD_FIDSUB(cpuf));
		return getcpuidfield_buf(dwBuf, cpuf);
	}

	// 取得CPU厂商（Vendor）
	//
	// result: 成功时返回字符串的长度（一般为12）。失败时返回0。
	// pvendor: 接收厂商信息的字符串缓冲区。至少为13字节。
	int cpu_getvendor(char* pvendor)
	{
		INT32 dwBuf[4];
		if (NULL == pvendor)	return 0;
		// Function 0: Vendor-ID and Largest Standard Function
		__cpuid(dwBuf, 0);
		// save. 保存到pvendor
		*(INT32*)&pvendor[0] = dwBuf[1];	// ebx: 前四个字符
		*(INT32*)&pvendor[4] = dwBuf[3];	// edx: 中间四个字符
		*(INT32*)&pvendor[8] = dwBuf[2];	// ecx: 最后四个字符
		pvendor[12] = '\0';
		return 12;
	}

	// 取得CPU商标（Brand）
	//
	// result: 成功时返回字符串的长度（一般为48）。失败时返回0。
	// pbrand: 接收商标信息的字符串缓冲区。至少为49字节。
	int cpu_getbrand(char* pbrand)
	{
		INT32 dwBuf[4];
		if (NULL == pbrand)	return 0;
		// Function 0x80000000: Largest Extended Function Number
		__cpuid(dwBuf, 0x80000000);
		if (dwBuf[0] < 0x80000004)	return 0;
		// Function 80000002h,80000003h,80000004h: Processor Brand String
		__cpuid((INT32*)&pbrand[0], 0x80000002);	// 前16个字符
		__cpuid((INT32*)&pbrand[16], 0x80000003);	// 中间16个字符
		__cpuid((INT32*)&pbrand[32], 0x80000004);	// 最后16个字符
		pbrand[48] = '\0';
		return 48;
	}


	// 是否支持MMX指令集
	BOOL	simd_mmx(BOOL* phwmmx)
	{
		const INT32	BIT_D_MMX = 0x00800000;	// bit 23
		BOOL	rt = FALSE;	// result
		INT32 dwBuf[4];

		// check processor support
		__cpuid(dwBuf, 1);	// Function 1: Feature Information
		if (dwBuf[3] & BIT_D_MMX)	rt = TRUE;
		if (NULL != phwmmx)	*phwmmx = rt;

		// check OS support
		if (rt)
		{
#if defined(_WIN64)
			// VC编译器不支持64位下的MMX。
			rt = FALSE;
#else
			__try
			{
				_mm_empty();	// MMX instruction: emms
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				rt = FALSE;
			}
#endif	// #if defined(_WIN64)
		}

		return rt;
	}

	// 检测SSE系列指令集的支持级别
	int	simd_sse_level(int* phwsse)
	{
		const INT32	BIT_D_SSE = 0x02000000;	// bit 25
		const INT32	BIT_D_SSE2 = 0x04000000;	// bit 26
		const INT32	BIT_C_SSE3 = 0x00000001;	// bit 0
		const INT32	BIT_C_SSSE3 = 0x00000100;	// bit 9
		const INT32	BIT_C_SSE41 = 0x00080000;	// bit 19
		const INT32	BIT_C_SSE42 = 0x00100000;	// bit 20
		int	rt = SIMD_SSE_NONE;	// result
		INT32 dwBuf[4];

		// check processor support
		__cpuid(dwBuf, 1);	// Function 1: Feature Information
		if (dwBuf[3] & BIT_D_SSE)
		{
			rt = SIMD_SSE_1;
			if (dwBuf[3] & BIT_D_SSE2)
			{
				rt = SIMD_SSE_2;
				if (dwBuf[2] & BIT_C_SSE3)
				{
					rt = SIMD_SSE_3;
					if (dwBuf[2] & BIT_C_SSSE3)
					{
						rt = SIMD_SSE_3S;
						if (dwBuf[2] & BIT_C_SSE41)
						{
							rt = SIMD_SSE_41;
							if (dwBuf[2] & BIT_C_SSE42)
							{
								rt = SIMD_SSE_42;
							}
						}
					}
				}
			}
		}
		if (NULL != phwsse)	*phwsse = rt;

		// check OS support
		__try
		{
			__m128 xmm1 = _mm_setzero_ps();	// SSE instruction: xorps
			if (0 != *(int*)&xmm1)	rt = SIMD_SSE_NONE;	// 避免Release模式编译优化时剔除上一条语句
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			rt = SIMD_SSE_NONE;
		}

		return rt;
	}

	// 检测AVX系列指令集的支持级别.
	int	simd_avx_level(int* phwavx)
	{
		int	rt = SIMD_AVX_NONE;	// result

		// check processor support
		if (0 != getcpuidfield(CPUF_AVX))
		{
			rt = SIMD_AVX_1;
			if (0 != getcpuidfield(CPUF_AVX2))
			{
				rt = SIMD_AVX_2;
			}
		}
		if (NULL != phwavx)	*phwavx = rt;

		// check OS support
		if (0 != getcpuidfield(CPUF_OSXSAVE))	// XGETBV enabled for application use.
		{
			UINT32 n = getcpuidfield(CPUF_XFeatureSupportedMaskLo);	// XCR0: XFeatureSupportedMask register.
			if (6 == (n & 6))	// XCR0[2:1] = ‘11b’ (XMM state and YMM state are enabled by OS).
			{
				return rt;
			}
		}
		return SIMD_AVX_NONE;
	}

	int main_bak(int argc, _TCHAR* argv[])
	{
		int i;

		//__cpuidex(dwBuf, 0,0);
		//__cpuid(dwBuf, 0);
		//printf("%.8X\t%.8X\t%.8X\t%.8X\n", dwBuf[0],dwBuf[1],dwBuf[2],dwBuf[3]);

		cpu_getvendor(szBuf);
		printf("CPU Vendor:\t%s\n", szBuf);

		cpu_getbrand(szBuf);
		printf("CPU Name:\t%s\n", szBuf);

		BOOL bhwmmx;	// 硬件支持MMX.
		BOOL bmmx;	// 操作系统支持MMX.
		bmmx = simd_mmx(&bhwmmx);
		printf("MMX: %d\t// hw: %d\n", bmmx, bhwmmx);

		int	nhwsse;	// 硬件支持SSE.
		int	nsse;	// 操作系统支持SSE.
		nsse = simd_sse_level(&nhwsse);
		printf("SSE: %d\t// hw: %d\n", nsse, nhwsse);
		for (i = 1; i < sizeof(simd_sse_names) / sizeof(simd_sse_names[0]); ++i)
		{
			if (nhwsse >= i)	printf("\t%s\n", simd_sse_names[i]);
		}

		// test SSE4A/AES/PCLMULQDQ
		printf("SSE4A: %d\n", getcpuidfield(CPUF_SSE4A));
		printf("AES: %d\n", getcpuidfield(CPUF_AES));
		printf("PCLMULQDQ: %d\n", getcpuidfield(CPUF_PCLMULQDQ));

		// test AVX
		int	nhwavx;	// 硬件支持AVX.
		int	navx;	// 操作系统支持AVX.
		navx = simd_avx_level(&nhwavx);
		printf("AVX: %d\t// hw: %d\n", navx, nhwavx);
		for (i = 1; i < sizeof(simd_avx_names) / sizeof(simd_avx_names[0]); ++i)
		{
			if (nhwavx >= i)	printf("\t%s\n", simd_avx_names[i]);
		}

		// test F16C/FMA/FMA4/XOP
		printf("F16C: %d\n", getcpuidfield(CPUF_F16C));
		printf("FMA: %d\n", getcpuidfield(CPUF_FMA));
		printf("FMA4: %d\n", getcpuidfield(CPUF_FMA4));
		printf("XOP: %d\n", getcpuidfield(CPUF_XOP));

		return 0;
	}
	/// <summary>
	/// 监测环境CPU是否支持PaddleOCR.
	/// </summary>
	/// <returns>0:代表CPU不支持AVX指令集，1代表CPU支持AVX指令集,2代表CPU支持AVX2指令集</returns>
	int IsCPUSupport_AVX()
	{
		int i;
		cpu_getvendor(szBuf);
		//printf("CPU Vendor:\t%s\n", szBuf);

		cpu_getbrand(szBuf);
		//printf("CPU Name:\t%s\n", szBuf);

		BOOL bhwmmx;	// 硬件支持MMX.
		BOOL bmmx;	// 操作系统支持MMX.
		bmmx = simd_mmx(&bhwmmx);
		//printf("MMX: %d\t// hw: %d\n", bmmx, bhwmmx);

		int	nhwsse;	// 硬件支持SSE.
		int	nsse;	// 操作系统支持SSE.
		nsse = simd_sse_level(&nhwsse);
		//printf("SSE: %d\t// hw: %d\n", nsse, nhwsse);
		printf("CPU Instruction set:");
		for (i = 1; i < sizeof(simd_sse_names) / sizeof(simd_sse_names[0]); ++i)
		{
			if (nhwsse >= i)	printf("%s,", simd_sse_names[i]);
		}

		// test SSE4A/AES/PCLMULQDQ
		//printf("SSE4A: %d\n", getcpuidfield(CPUF_SSE4A));
		//printf("AES: %d\n", getcpuidfield(CPUF_AES));
		//printf("PCLMULQDQ: %d\n", getcpuidfield(CPUF_PCLMULQDQ));

		// test AVX
		int	nhwavx;	// 硬件支持AVX.
		int	navx;	// 操作系统支持AVX.
		navx = simd_avx_level(&nhwavx);
		
	//	printf("AVX: %d\t// hw: %d\n", navx, nhwavx);
		for (i = 1; i < sizeof(simd_avx_names) / sizeof(simd_avx_names[0]); ++i)
		{
			if (nhwavx >= i)	printf("%s,", simd_avx_names[i]);
		}
		printf("\n");
		return nhwavx;//0：不支持，1：AVX，2：AVX2
		// test F16C/FMA/FMA4/XOP
		//printf("F16C: %d\n", getcpuidfield(CPUF_F16C));
		//printf("FMA: %d\n", getcpuidfield(CPUF_FMA));
		//printf("FMA4: %d\n", getcpuidfield(CPUF_FMA4));
		//printf("XOP: %d\n", getcpuidfield(CPUF_XOP));
	}
}

#endif