# Ray tracing
This project includes a rendercore for lighthouse 2. It can render using whitted style ray tracing or path tracing.
 
### Visual studio
The project can be opened in Visual studio, the custom render core however has not been added and will need to be added manually.
In case of problems one can always contact for a demo.

### Using cmake
One can build with:

`cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DOptiX_INSTALL_DIR:PATH=/opt/optix-6 -DOptiX7_INSTALL_DIR:PATH=/opt/optix -B build && make -j$(nproc --all) -C build`

And run with:

`(cd apps/tinyapp && ../../build/apps/tinyapp/TinyApp)
`