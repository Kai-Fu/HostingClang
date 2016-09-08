#include <stdio.h>
#include <math.h>

struct float3 
{
	float x;
	float y;
	float z;
	
	float3 operator + (const float3& val) const {
		float3 ret = *this;
		ret.x += val.x;
		ret.y += val.y;
		ret.z += val.z;
		return ret;
	}
};

extern "C" {
	float3 MyTestFunc(const float3& a, const float3& b);
	void testExtFunc(float3& ref);
}



float3 MyTestFunc(const float3& a, const float3& b)
{
	// During JIT the CRT function can be resolved by looking up in the exported symbol of hosting side.
	float sinVal = sinf(a.x);
	printf("Printout from hosted C++ : %f\n", sinVal);
	float3 val(a);
	testExtFunc(val);
    return a + b;
}