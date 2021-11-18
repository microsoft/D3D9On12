## Unit Tests
Unit testing for d3d9on12 is done with the Google Test (gtest) and Google Mock (gmock) frameworks. By default the tests will be included in the solution, but if for some reason you don't want to include them you can opt out at solution generation time by adding `-DBUILD_TESTS=OFF` to your cmake command.

## Running tests
* If you [add the Google Test adapter](https://docs.microsoft.com/en-us/visualstudio/test/how-to-use-google-test-for-cpp?view=vs-2022) component to Visual Studio you can run the tests from within the IDE's Test Explorer window (CTRL+E, T). 
* You can also run `d3d9on12_txt.exe` directly.

## Testing as part of PR cycle
Using GitHub actions we should be able to build 9on12 and run the unit tests against them to validate changes. Looks like we'll need to add a script to install the Windows SDK and WDK as well as pull down the nuget packages in order to build.

As of right now, there aren't any meaningful unit tests here, but as time allows I'll be adding the ability to mock different components and that will open the ability to add tests.