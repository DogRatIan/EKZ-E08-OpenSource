@Echo off
..\EkzPack2.exe Output\Exe\spi-generic.bin Output\Exe\spi-generic.ekz
copy /v Output\Exe\spi-generic.ekz ..\..\qt\asset
REM "I:\working\ekz_e08\eclipse_msw\EkzPack2\Debug\EkzPack2_debug.exe" Output\Exe\spi-generic.bin Output\Exe\spi-generic.ekz
