
# Create Project Folder
Run the export script located in your ESP-IDF installation directory (which is typically ~/esp/esp-idf):

```
    cd ~/esp/esp-idf
    . ./export.sh
```

# Scaffold the Project
Use the ESP-IDF command line to generate the standard boilerplate.
```
    idf.py create-project xiao_blink
    cd xiao_blink
```
This creates a root CMakeLists.txt 

# Set the Hardware Target
ESP-IDF supports many different Espressif chips. You must tell the build system to configure the compiler and FreeRTOS settings specifically for the C6.
```
idf.py set-target esp32c6
```

# Write the Application Code
Open main/xiao_blink.c and replace its contents with the code.