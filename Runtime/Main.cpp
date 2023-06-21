#include "Application/Application.h"
#include "Shader/ShaderDependency.h"

void Test() {
	ShaderDependency dependency("Test.hlsl");
    
}

int main() {
    Test();
    Application app;
    RunApplication(app);
}