# Releasing a new version of Cesium for O3DE

This is the process we follow when releasing a new version of Cesium for O3DE on GitHub.

## Test the release candidate

* Verify that the cesium-native submodule in the `External` directory references the expected commit of cesium-native. Update it if necessary. Verify that CI has completed successfully for that commit of cesium-native.
* Wait for CI to complete for the main branch. Verify that it does so successfully.
* Remove any existing copy of the Cesium for O3DE plugin from the engine plugins directory on your system.
* Download the `CesiumForO3DE` for the `main` branch of cesium-o3de. Extract it to a new directory. 
* Clone a fresh copy of [cesium-o3de-samples](https://github.com/CesiumGS/cesium-o3de-samples) to a new directory. Launch the O3DE Project Manager and open `CesiumForO3DESamples` project.
* Open each level and verify it works correctly:
  * Does it open without crashing?
  * Does the engine give out any warning messages for Cesium components both in the dialog or console? 
  * Open the Script Canvas editor and make sure all the scripts work correctly.
  * Does it look correct?
  * Press Play. Does it work as expected? The billboard in each level should give you a good idea of what to expect.

If all of the above goes well, you're ready to release Cesium for O3DE.

## Update CHANGES.md and tag the cesium-o3de releases

While doing the steps below, make sure no new changes are going into either cesium-o3de or cesium-native that may invalidate the testing you did above. If new changes go in, it's ok, but you should either retest with those changes or make sure they are not included in the release.

* Change the version in `gem.json`:
  * Change the `version` property to the new three digit, dot-delimited version number. Use [Semantic Versioning](https://semver.org/) to pick the version number.
* Change the `version` property in `package.json` to match the `version` above.
* Verify that cesium-o3de's CHANGES.md is complete and accurate.
* Verify again that cesium-native CI has completed successfully on all platforms.
* Verify again that the submodule reference in cesium-o3de references the correction commit of cesium-native.
* Verify again that cesium-o3de CI has completed successfully on all platforms.
* Tag the cesium-o3de release, e.g., `git tag -a v1.1.0 -m "1.1.0 release"`
* Push the tag to github: `git push origin v1.1.0`

# Publish the release on GitHub

* Wait for the release tag CI build to complete.
* Download the tag's `CesiumForO3DE`. You can find it by switching to the tag in the GitHub UI, clicking the green tick in the header, and then clicking the Details button next to `CesiumForO3DE`. 
* Create a new release on GitHub: https://github.com/CesiumGS/cesium-o3de/releases/new. Copy the changelog into it. Follow the format used in previous release. Upload the release ZIP that you downloaded above.

# Update Cesium for O3DE Samples

Assuming you tested the release candidate as described above, you should have [cesium-o3de-samples](https://github.com/CesiumGS/cesium-o3de-samples) using the updated plugin. You'll use this to push updates to the project.

## Update ion Access Tokens and Project

1. Create a new branch of cesium-o3de-samples. 
2. Delete the Cesium for O3DE Samples token for the release before last, which should expire close to the present date.
3. Create a new access token using the CesiumJS ion account. 
   * The name of the token should match "Cesium for O3DE Samples x.x.x - Delete on September 1st, 2021". The expiry date should be two months later than present. 
   * The scope of the token should be "assets:read" for all assets.
4. Copy the access token you just created and update each tilesets in each level. 
5. Open cesium-o3de-samples in O3DE Engine.
6. Visit every scene again to make sure that the view is correct and that nothing appears to be missing. 
7. Commit and push your changes. Create a PR to merge to `main` and tag a reviewer.

## Publish the Cesium for O3DE Samples release on GitHub

After the update has been merged to `main`, do the following:
1. Pull and check out the latest version of `main` from GitHub, and then tag the new release by doing the following:
  * `git tag -a v1.10.0 -m "v1.10.0 release"`
  * `git push origin v1.10.0`
2. Create a new release on GitHub: https://github.com/CesiumGS/cesium-o3de-samples/releases/new. Select the tag you created above. Add a changelog in the body to describe recent updates. Follow the format used in previous release. 
3. Publish the release.