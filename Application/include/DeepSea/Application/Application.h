/*
 * Copyright 2017 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Application/Export.h>
#include <DeepSea/Application/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for working with an application.
 *
 * An application provides an interface to create and destroy windows, update and draw the
 * application, and respond to events. The underlying implementation may differ, such as SDL,
 * Qt, or a platform-specifc GUI API.
 *
 * @see dsApplication
 */

/**
 * @brief Adds a window responder to an application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param responder The responder to add.
 * @return The ID of the responder, or 0 if an error occurred.
 */
DS_APPLICATION_EXPORT uint32_t dsApplication_addWindowResponder(dsApplication* application,
	const dsWindowResponder* responder);

/**
 * @brief Removes a window responder from an application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param responderId The ID of the responder to remove.
 * @return True if the responder was removed.
 */
DS_APPLICATION_EXPORT bool dsApplication_removeWindowResponder(dsApplication* application,
	uint32_t responderId);

/**
 * @brief Adds an event responder to an application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param responder The responder to add.
 * @return The ID of the responder, or 0 if an error occurred.
 */
DS_APPLICATION_EXPORT uint32_t dsApplication_addEventResponder(dsApplication* application,
	const dsEventResponder* responder);

/**
 * @brief Removes an event responder from an application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param responderId The ID of the responder to remove.
 * @return True if the responder was removed.
 */
DS_APPLICATION_EXPORT bool dsApplication_removeEventResponder(dsApplication* application,
	uint32_t responderId);

/**
 * @brief Sets the update function for the application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param function The udpate function.
 * @param userData The user data to provide to the function.
 * @return True if the function was set.
 */
DS_APPLICATION_EXPORT bool dsApplication_setUpdateFunction(dsApplication* application,
	dsUpdateApplicationFunction function, void* userData);

/**
 * @brief Creates a new window and adds it to the application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param title The title of the window.
 * @param position The position of the window in screen space, or NULL if the default position.
 *     If flags contains the dsWindowFlags_Center flag, then the x coordinate of the position is the
 *     display index.
 * @param width The width of the window.
 * @param height The height of the window.
 * @param flags Combination of dsWindowFlags values.
 * @return The created window or NULL if an error occurred.
 */
DS_APPLICATION_EXPORT dsWindow* dsApplication_createWindow(dsApplication* application,
	const char* title, const dsVector2i* position, uint32_t width, uint32_t height, int flags);

/**
 * @brief Adds an existing window to the application.
 *
 * This is useful when more control is required when setting up a window. For example, if it's
 * actually a sub-element of a surrounding window that also contains widgets.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param window The window to add.
 * @return True if the window was added.
 */
DS_APPLICATION_EXPORT bool dsApplication_addWindow(dsApplication* application, dsWindow* window);

/**
 * @brief Removes a window from the application without destroying it.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param window The window to remove.
 * @return True if the window was removed.
 */
DS_APPLICATION_EXPORT bool dsApplication_removeWindow(dsApplication* application, dsWindow* window);

/**
 * @brief Runs the application.
 * @param application The application.
 * @return The error code of the application. This is intended to be returned from main().
 */
DS_APPLICATION_EXPORT int dsApplication_run(dsApplication* application);

/**
 * @brief Quits the application.
 * @remark The application may not quit until the next iteration of the update loop.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param exitCode The exit code for the application.
 * @return True
 */
DS_APPLICATION_EXPORT bool dsApplication_quit(dsApplication* application, int exitCode);

/**
 * @brief Gets the cursor used by the application.
 * @param application The application.
 * @return The cursor.
 */
DS_APPLICATION_EXPORT dsCursor dsApplication_getCursor(const dsApplication* application);

/**
 * @brief Sets the cursor used by the application.
 * @remark errno will be set on failure.
 * @param application The application.
 * @param cursor The new cursor type.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_setCursor(dsApplication* application, dsCursor cursor);

/**
 * @brief Gets whether or not the cursor is hidden.
 *
 * This is used for the OS-provided cursor. For example, the hardware cursor could be hidden and a
 * texture shown for the cursor instead.
 *
 * @param application The application.
 * @return True if the cursor is hidden.
 */
DS_APPLICATION_EXPORT bool dsApplication_getCursorHidden(const dsApplication* application);

/**
 * @brief Sets whether or not the cursor is hidden.
 *
 * This is used for the OS-provided cursor. For example, the hardware cursor could be hidden and a
 * texture shown for the cursor instead.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param hidden True if the cursor is hidden.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_setCursorHidden(dsApplication* application, bool hidden);

/**
 * @brief Gets whether or not a keyboard key is currently pressed.
 * @param application The application.
 * @param key The key to check.
 * @return True if the key is pressed.
 */
DS_APPLICATION_EXPORT bool dsApplication_isKeyPressed(const dsApplication* application,
	dsKeyCode key);

/**
 * @brief Gets the currently pressed key modifiers.
 * @param application The application.
 * @return The currently pressed modifiers.
 */
DS_APPLICATION_EXPORT dsKeyModifier dsApplication_getKeyModifiers(const dsApplication* application);

/**
 * @brief Begins accepting text input.
 * @remark errno will be set on failure.
 * @param application The application.
 * @return False if input couldn't be begun.
 */
DS_APPLICATION_EXPORT bool dsApplication_beginTextInput(dsApplication* application);

/**
 * @brief Function for ending text input.
 * @remark errno will be set on failure.
 * @param application The application.
 * @return False if input couldn't be ended.
 */
DS_APPLICATION_EXPORT bool dsApplication_endTextInput(dsApplication* application);

/**
 * @brief Setting the editing rectangle for editing text.
 *
 * This is generally used for suggestions for unicode entry.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param rect The renctangle to edit text in.
 * @return False if the input rectangle couldn't be set.
 */
DS_APPLICATION_EXPORT bool dsApplication_setTextInputRect(dsApplication* application,
	const dsAlignedBox2i* rect);

/**
 * @brief Gets the current position of the mouse.
 * @remark errno will be set on failure.
 * @param[out] outPosition The position of the mouse in screen coordinates.
 * @param application The application.
 * @return False if the position couldn't be queried.
 */
DS_APPLICATION_EXPORT bool dsApplication_getMousePosition(dsVector2i* outPosition,
	const dsApplication* application);

/**
 * @brief Sets the current position of the mouse.
 *
 * This may generate a mouse move event.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @param window The window to set the mouse position relative to. If NULL, it will be in screen
 *     coordinates.
 * @param position The position of the mouse.
 */
DS_APPLICATION_EXPORT bool dsApplication_setMousePosition(dsApplication* application,
	dsWindow* window, const dsVector2i* position);

/**
 * @brief Gets the currently pressed mouse buttons.
 * @param application The application.
 * @return A bitmask of the currently pressed mouse buttons.
 */
DS_APPLICATION_EXPORT uint32_t dsApplication_getPressedMouseButtons(
	const dsApplication* application);

/**
 * @brief Gets the window with focus.
 * @param application The application.
 * @return The window with focus, or NULL if no window has focus.
 */
DS_APPLICATION_EXPORT dsWindow* dsApplication_getFocusWindow(const dsApplication* application);

/**
 * @brief Dispatches an event through the application.
 *
 * This is usually called by the application, but could also be used to generate fake events.
 *
 * @param application The application.
 * @param window The window that the event originated fromm.
 * @param event The event to send.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_dispatchEvent(dsApplication* application, dsWindow* window,
	const dsEvent* event);

/**
 * @brief Initializes the members of an application.
 *
 * This will initiialize all members to 0 and set up any internal structures. This is called by the
 * application implementation.
 *
 * @remark errno will be set on failure.
 * @param application The application.
 * @return False if an error occurred.
 */
DS_APPLICATION_EXPORT bool dsApplication_initialize(dsApplication* application);

/**
 * @brief Destroys the private members of an application.
 *
 * This is called by the application implementation. The following pointers are freed:
 * - windowResponders
 * - eventResponders
 * - windows
 * Only the base pointers are destroyed, not the elements within the arrays.
 *
 * @param application The application.
 */
DS_APPLICATION_EXPORT void dsApplication_shutdown(dsApplication* application);

#ifdef __cplusplus
}
#endif