# d3d11book



1. Remove old collision header and source from the filter (for projects that need collision, use DirectXCollision)
2. Convert the project from 32-bit to 64-bit for both release and debug build (PIX 2206.20 requires 64-bit applications)
3. Remove `d3dx11d.lib` from debug build. Remove `d3dx11.lib` from release build. Remove `dxerr.lib` from both debug and release build. 
4. Add additional include and library direction:  `../../Common`
5. Add `DDSTextureLoader.cpp/h` `dxerr.cpp/h`to the common filter
6. Add `using namspace DirectX` and `using namespace PackedVector;` whenever necessary
7. Replace `D3DX11CreateShaderResourceViewFromFile` or `CreateTexture2DArraySRV` with 

```c++
ID3D11Resource* texResource = nullptr;
HR(DirectX::CreateDDSTextureFromFile(md3dDevice, L"Textures/grass.dds", &texResource, &mGrassMapSRV));
ReleaseCOM(texResource); // view saves reference
```

