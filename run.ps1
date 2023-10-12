try {
    $PSDefaultParameterValues = @{'Out-File:Encoding' = 'ascii'}

    Set-Location $PSScriptRoot/build/
    cmake.exe --build . --config Debug --target handGestureUserStudy
    Set-Location $PSScriptRoot/build/Debug/
    ./handGestureUserStudy
} finally {
    # return to project root directory even if CTRL+C was used
    Set-Location $PSScriptRoot
}