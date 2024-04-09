# DeepFacetX
Implementation of "Microfacet theory for non-uniform heightfields" in Autodesk Maya Arnold plugin

## Build

#### Setting environment variables

- `ARNOLD_PATH` to `C:\Program Files\Autodesk\Arnold\maya2023`

- `ARNOLD_PLUGIN_PATH` to `your plugin folder`, in this case is `/plugin` of this project
- `MTOA_TEMPLATES_PATH` the same as `ARNOLD_PLUGIN_PATH`
- Add `%ARNOLD_PATH%\bin` to your system's `PATH` variable

#### Build Commands

`mkdir build & cd build & cmake ..`

Then use your own tool to compile the project.

After generated the `.dll` file, make sure to copy the file under `/plugin`
