Pebble grouped timers
=====================
A timer app for Pebble that allows you to group timers into lists. Timer groups
can be set to progress automatically, or wait for user input before starting the
next timer. There is no limit on the number of timers you can create (except for
the implicit defined limit by the amount of persistent storage Pebble allocates
to each app).


## Alpha warning
This app is currently in alpha. The UI is incomplete, not all planned features
are implemented, and there are probably plenty of bugs waiting to be found.
However, please feel free to try the app (and let me know of any bugs you find
or features you want).


## Installation
This app is not yet available on the app store, so the app must be installed
manually.

### CloudPebble
If you don't already have the Pebble SDK setup, the easiest way to install this
app is to import the GitHub project into
[CloudPebble](https://cloudpebble.net/):

1. Login to [CloudPebble](https://cloudpebble.net/).
2. Click "Import".
3. Select "Import from GitHub".
4. Fill in the fields, using the following url for the GitHub project field:
https://github.com/spencewenski/pebble_grouped_timers
5. Click "Import"
6. Open the compilation window by clicking the "Compilation" tab in the sidebar.
7. Click "Run Build" to build the project.
8. Select the "Phone" tab of the "Compilation" window.
9. Turn on the developer connection in your phone's Pebble app.
10. Click "Install and Run".

### Pebble SDK
If you already have the Pebble SDK installed, or you're not intimidated by the
term "SDK", you can also install the app using the SDK.

1. Install the SDK following the instructions [here](https://developer.pebble.com/sdk/).
2. Clone this repo:

    ```bash
    git clone https://github.com/spencewenski/pebble_grouped_timers.git
    ```

3. Build the app

    ```bash
    # Default 'make' creates a debug build
    make
    # Or you can create a release build
    make release
    ```

4. Install the app

    ```bash
    # Install on an emulator
    pebble install --emulator emery
    # Install on your watch
    pebble install --phone 1.2.3.4
    ```


## Navigating the incomplete UI
### How do I create a timer?
1. In the Main window, either:
  a. select a timer group from the list, or
  b. create a timer group by selecting the "New Group" option
2. Select the "New Timer" option
3. Edit the

### How do I use the Timer Edit window?
The Timer Edit window currently doesn't visually indicate which field is
selected. Other than that, the Timer Edit window behaves almost exactly the
same as the official Pebble timer app. The only difference is that clicking the
back button will commit the timer if the timer has a non-zero field.

### How do I get to the Timer Edit window?
There are two ways:

1. Long press the select button when in the Timer Group window.
2. Single press the down button when in the Timer Countdown window.

### How do I delete a timer?
1. Get to the edit window using one of the methods described above.
2. Set all the timer values to zero.
3. Commit the timer by pressing back or select until the app returns to the
Timer Group window.


## Features
### Repeat styles
How to repeat when a timer elapses.
- None: Don't repeat. Continue to the next timer, but don't repeat the group.
- Single: Repeat the current timer.
- Group: Continue to the next timer, and repeat the group when the last timer
elapses.
- Use app settings: *NOT IMPLEMENTED YET* Use the global app settings.

### Progress styles
How to progress when a timer elapses.
- Don't start next: When the timer is stopped, reset and stay on the current
timer. Don't reset the timer.
- Auto start next: Automatically continue to the next timer when the current
timer elapses.
- Wait for user: Wait for the user to stop the current timer before
automatically continuing to the next timer.
- Use app settings: *NOT IMPLEMENTED YET* Use the global app settings.

### Vibrate styles
How to vibrate when a timer elapses.
- Don't vibrate: *NOT IMPLEMENTED YET* Don't vibrate.
- Nudge: Short pulse every minute until the timer is stopped.
- Continuous: *NOT IMPLEMENTED YET* Short pulse every few seconds until the
timer is stopped.
- Use app settings: *NOT IMPLEMENTED YET* Use the global app settings.
