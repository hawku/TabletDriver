using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows;

namespace TabletDriverGUI
{
    public partial class App : Application
    {
        // Single Instance GUID
        public static Guid InstanceGuid = new Guid("30fa6b68-de2b-4a43-a9f1-6b23f4ba2b56");
        Mutex instanceMutex;

        public App()
        {

            //
            // Prevent multiple instances of TabletDriverGUI
            //
            instanceMutex = new Mutex(true, "Local\\" + InstanceGuid.ToString());
            if (instanceMutex.WaitOne(TimeSpan.Zero, true))
            {

                //
                // Kill old TabletDriverService.exe instances
                //
                Process[] processes = Process.GetProcessesByName("TabletDriverService");
                foreach (Process process in processes)
                {
                    if (process.ProcessName == "TabletDriverService")
                    {
                        try
                        {
                            process.Kill();
                        }
                        catch (Exception)
                        {
                            MessageBox.Show(
                                "You have another TabletDriverService.exe running in the background. " +
                                "Please shutdown that before starting the GUI!",
                                "Error!", MessageBoxButton.OK, MessageBoxImage.Error
                            );
                            instanceMutex.ReleaseMutex();
                            Shutdown();
                            return;
                        }
                    }
                }

                //
                // Other tablet driver processes
                //
                string[] tabletDriverProcessNames =
                {

                    // Wacom
                    "Pen_Tablet",
                    "Wacom_Tablet",

                    // XP-Pen
                    "PentabletService",
                    "Pentablet",

                    // VEIKK
                    "TabletDriverCenter",
                    "TabletDriverSetting",

                    // Huion
                    "Huion Tablet"

                };


                //
                // Find driver processes
                //
                processes = Process.GetProcesses();
                List<Process> foundProcesses = new List<Process>();
                foreach (Process process in processes)
                {
                    foreach (string processName in tabletDriverProcessNames)
                    {
                        if (process.ProcessName.ToLower() == processName.ToLower())
                        {
                            foundProcesses.Add(process);
                        }
                    }
                }

                //
                // Try to kill driver processes
                //
                foreach (Process process in foundProcesses)
                {
                    try
                    {
                        process.Kill();
                        Thread.Sleep(100);
                    }
                    catch (Exception)
                    {
                        string processNames = "";
                        foreach (Process p in foundProcesses)
                        {
                            processNames += "- " + p.ProcessName + ".exe\n  ";
                        }

                        MessageBox.Show(
                            "You have other driver processes running:\n  " +
                            processNames +
                            "\nPlease shutdown those before starting this driver!",
                            "TabletDriverGUI - Error!", MessageBoxButton.OK, MessageBoxImage.Error
                        );
                        instanceMutex.ReleaseMutex();
                        Shutdown();
                        return;
                    }
                }


                MainWindow mainWindow = new MainWindow();
                mainWindow.Show();
                Exit += App_Exit;
            }
            else
            {
                MessageBox.Show("TabletDriverGUI is already open!");

                // Broadcast to the another instance to show itself
                NativeMethods.PostMessage(
                    (IntPtr)NativeMethods.HWND_BROADCAST,
                    NativeMethods.WM_SHOWTABLETDRIVERGUI,
                    IntPtr.Zero,
                    IntPtr.Zero
                );

                Shutdown();
            }
        }

        private void App_Exit(object sender, ExitEventArgs e)
        {
            try
            {
                instanceMutex.ReleaseMutex();
            }
            catch (Exception)
            {
            }
        }
    }
}
