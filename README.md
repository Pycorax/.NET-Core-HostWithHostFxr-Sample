# .NET Core Host with HostFxr Sample

This is a sample Visual Studio project for a x86-64 Windows native C++ application that is able to call functions in a .NET 5.0 library written in C#. The code here is based off Microsoft's sample here: https://github.com/dotnet/samples/tree/main/core/hosting/HostWithHostFxr

This was built in order to attempt to integrate the .NET Core runtime into a Windows-only game engine written in C++ that I am working on, hence, the Windows-only support and absence of Linux-relevant code when compared to Microsoft's sample.

This sample was built and tested on Visual Studio 2019 with the following workloads:
- Desktop development with C++
- .NET cross-platform development

I was unable to install the `Microsoft.NETCore.DotNetAppHost` package from nuget so instead, I had manually downloaded the corresponding runtime (`runtime.win-x64.Microsoft.NETCore.DotNetAppHost`) for it and extracted it out into the `Native.NETHostFxr/lib/netcore` folder. The version used is [5.0.6](https://www.nuget.org/packages/runtime.win-x64.Microsoft.NETCore.DotNetAppHost/5.0.6).