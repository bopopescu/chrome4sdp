### WebRefiner
WebRefiner is a content filtering component for SWE Browser. It provides a core filtering engine for filtering URLs and a DOM manipulator for applying cosmetic filters on web pages to block advertisements, privacy info trackers and malware domains.

`TL;DR: you should change the subscription.url(s) in web_refiner_conf, and you're good to go.`

#### UI Enablement (in SWE Browser):
 * Default: toggle the option in Menu > Settings > Site settings > "WebRefiner"
 * Per-site (Overrides the default option on a per-website basis): toggle the option in Top-left Favicon Button > "WebRefiner"

#### Filter configuration:
 - The configuration file is in: **chrome/android/java/res_chromium/raw/web_refiner_conf**
 - By default **web_refiner_conf** contains no valid subscription URLs.
 - WebRefiner looks for the configuration only in the above path and failure to provide a valid configuration will result in a failed initialization of WebRefiner (and WebRefiner won't be shown in the UI)
 - WebRefiner is fully compatible with the *leading ad-blocking filter list format*. A wide variety of public filters can be configured in `subscriptions.url` or you can create your own compatible filter list.
 - Subscriptions configured in **web_refiner_conf** will always be downloaded from a http/https server at the very first browser startup and will be updated periodically according their expiration.
 - All the downloaded filter files (and internal WebRefiner data) are stored in device `/data/data/<browser-application>/web_refiner/`. Deleting the folder, or clearing the APP DATA will bring WebRefiner back to its default state.
 - Filters are applied in the same order as in the configuration file.

#### WebRefiner libraries:
 - The libraries listed in this repository are built on top of SWE based on Chromium version M46. Attempting to use these libraries in any other version of chromium will result in instability.
 - `libswewebrefiner.so` is built with dependency on `libswe.so`. If you are changing the chromium library name to something else in your project, then `libswewebrefiner.so` library loading will simply fail at runtime.
 - Libraries support ARM (32-bit) architecture only.

#### Browser command line arguments:
```
--disable-web-refiner               disables WebRefiner completely
--enable-web-refiner-logcat-stats   enables logcat output of applied filters
```

#### Debugging: Logcat messages:
```
logcat -s WebRefiner       generic WebRefiner failure messages
logcat -s WebRefinerStat   filter stats when '--enable-web-refiner-logcat-stats' is used
```
