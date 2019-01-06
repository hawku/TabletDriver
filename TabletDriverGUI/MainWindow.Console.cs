using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TabletDriverGUI
{
    public partial class MainWindow : Window
    {

        //
        // Console buffer to text
        //
        private void ConsoleBufferToText()
        {
            StringBuilder stringBuilder = new StringBuilder();

            if (driver == null) return;

            // Lock console
            driver.ConsoleLock();

            // Get console status
            if (!driver.HasConsoleUpdated)
            {
                driver.ConsoleUnlock();
                return;
            }
            driver.HasConsoleUpdated = false;

            // Create a string from buffer
            foreach (string line in driver.ConsoleBuffer)
            {
                stringBuilder.Append(line);
                stringBuilder.Append("\r\n");
            }

            // Unlock console
            driver.ConsoleUnlock();

            // Set output
            textConsole.Text = stringBuilder.ToString();

            // Scroll to end
            scrollConsole.ScrollToEnd();

        }


        //
        // Search text from rows
        //
        private List<string> SearchRows(List<string> rows, string search, int rowsBefore, int rowsAfter)
        {
            List<string> buffer = new List<string>(rowsBefore);
            List<string> output = new List<string>();
            int rowCounter = 0;

            foreach (string row in rows)
            {
                if (row.Contains(search))
                {
                    if (buffer.Count > 0)
                    {
                        foreach (string bufferLine in buffer)
                        {
                            output.Add(bufferLine);
                        }
                        buffer.Clear();
                    }
                    output.Add(row.Trim());
                    rowCounter = rowsAfter;
                }
                else if (rowCounter > 0)
                {
                    output.Add(row.Trim());
                    rowCounter--;
                }
                else
                {
                    buffer.Add(row);
                    if (buffer.Count > rowsBefore)
                    {
                        buffer.RemoveAt(0);
                    }
                }
            }
            return output;
        }


        //
        // Send a command to driver
        //
        private void ConsoleSendCommand(string line)
        {
            if (commandHistory.Last<string>() != line)
            {
                commandHistory.Add(line);
            }
            commandHistoryIndex = commandHistory.Count();
            textConsoleInput.Text = "";
            textConsoleInput.ScrollToEnd();
            try
            {
                driver.SendCommand(line);
            }
            catch (Exception e)
            {
                driver.ConsoleAddLine("Error! " + e.Message);
            }
        }


        //
        // Console update timer tick
        //
        private void TimerConsoleUpdate_Tick(object sender, EventArgs e)
        {
            
            // Update console text if console tab is active
            if (tabControl.SelectedItem == tabConsole && WindowState != WindowState.Minimized)
            {
                ConsoleBufferToText();
            }
        }


        //
        // Console input key down
        //
        private void TextConsoleInput_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                string line = textConsoleInput.Text;
                ConsoleSendCommand(line.Trim());
            }
        }


        //
        // Console input preview key down
        //
        private void TextConsoleInput_PreviewKeyDown(object sender, KeyEventArgs e)
        {

            //
            // Command tab complete
            //
            if (e.Key == Key.Tab
                ||
                (e.Key == Key.Space && e.KeyboardDevice.Modifiers == ModifierKeys.Control)
            )
            {
                string fill;
                string inputText = textConsoleInput.Text.Trim().ToLower();
                if (inputText.StartsWith("help "))
                {
                    fill = driver.CompleteCommandName(inputText.Substring(5), true);
                    if (fill != null)
                        textConsoleInput.Text = "Help " + fill;
                }
                else
                {
                    fill = driver.CompleteCommandName(inputText, true);
                    if (fill != null)
                        textConsoleInput.Text = fill;
                }
                if (fill != null)
                    textConsoleInput.CaretIndex = textConsoleInput.Text.Length;
                ConsoleBufferToText();
                e.Handled = true;
            }


            //
            // Up arrow
            //
            else if (e.Key == Key.Up)
            {
                commandHistoryIndex--;
                if (commandHistoryIndex < 0) commandHistoryIndex = 0;
                textConsoleInput.Text = commandHistory[commandHistoryIndex];
                textConsoleInput.CaretIndex = textConsoleInput.Text.Length;
            }

            //
            // Down arrow
            //
            else if (e.Key == Key.Down)
            {
                commandHistoryIndex++;
                if (commandHistoryIndex > commandHistory.Count() - 1)
                {
                    commandHistoryIndex = commandHistory.Count();
                    textConsoleInput.Text = "";
                }
                else
                {
                    textConsoleInput.Text = commandHistory[commandHistoryIndex];
                    textConsoleInput.CaretIndex = textConsoleInput.Text.Length;
                }
            }
        }


        //
        // Commands textbox key down
        //
        private void TextCommands_PreviewKeyDown(object sender, KeyEventArgs e)
        {

            //
            // Control + space command completion
            //
            if (
                e.Key == Key.Space && e.KeyboardDevice.Modifiers == ModifierKeys.Control
            )
            {
                TextBox textBoxSender = (TextBox)sender;

                string commandName = "";
                string text = textBoxSender.Text;

                int caretIndex = textBoxSender.CaretIndex;
                if (caretIndex >= text.Length)
                    caretIndex = text.Length - 1;
                if (caretIndex < 0) caretIndex = 0;

                int startIndex = caretIndex;
                int endIndex = caretIndex;

                // Find word start index
                for (startIndex = caretIndex; startIndex > 0; startIndex--)
                {
                    // Find space or new line
                    if (text[startIndex] == '\r' || text[startIndex] == '\n' || text[startIndex] == ' ')
                    {
                        if (caretIndex == startIndex) continue;
                        startIndex++;
                        break;
                    }
                }

                // Find word end index
                caretIndex = caretIndex - 1;
                if (caretIndex < 0) caretIndex = 0;
                for (endIndex = caretIndex; endIndex < text.Length; endIndex++)
                {
                    // Find space or new line
                    if (text[endIndex] == '\r' || text[endIndex] == '\n' || text[endIndex] == ' ')
                    {
                        if (caretIndex == endIndex) continue;
                        break;
                    }
                }

                // Select word
                textBoxSender.SelectionStart = startIndex;
                textBoxSender.SelectionLength = endIndex - startIndex;

                // Command name completion
                commandName = textBoxSender.SelectedText.Trim();
                string completedCommand = driver.CompleteCommandName(commandName, false);
                string newText = text;

                // Set selected text as completed command name
                if (completedCommand != null)
                {

                    //
                    // Close old tool tip
                    //
                    if (textBoxSender.ToolTip != null)
                    {
                        ((ToolTip)textBoxSender.ToolTip).IsOpen = false;
                        ((ToolTip)textBoxSender.ToolTip).IsEnabled = false;
                    }
                    textBoxSender.SelectedText = completedCommand;

                    // Find commands
                    string foundCommands = "Commands:\n";
                    int commandCount = 1;
                    foreach (var command in driver.Commands)
                    {
                        if (command.Key.ToLower().StartsWith(completedCommand.ToLower()))
                        {
                            foundCommands += command.Value + " ";
                            if (commandCount % 10 == 0) foundCommands += "\n";
                        }

                        commandCount++;
                    }

                    //
                    // Create tool tip
                    //
                    ToolTip toolTip = new ToolTip
                    {
                        Placement = System.Windows.Controls.Primitives.PlacementMode.Relative,
                        PlacementTarget = textBoxSender,
                        HorizontalOffset = 100
                    };
                    toolTip.Opened += async delegate (object obj1, RoutedEventArgs eventArgs1)
                    {
                        toolTip.Content = foundCommands;
                        await Task.Delay(3000);
                        toolTip.IsOpen = false;
                        toolTip.IsEnabled = false;
                        textBoxSender.ToolTip = null;
                    };
                    toolTip.IsOpen = true;
                    textBoxSender.ToolTip = toolTip;

                }

                // Set cursor position
                textBoxSender.CaretIndex = textBoxSender.SelectionStart + textBoxSender.SelectionLength;

                // Reset selection
                textBoxSender.SelectionLength = 0;

                e.Handled = true;
            }
        }


        //
        // Console output context menu
        //
        private void ConsoleMenuClick(object sender, RoutedEventArgs e)
        {


            // Copy all
            if (sender == menuCopyAll)
            {
                Clipboard.SetText(textConsole.Text);
                SetStatus("Console output copied to clipboard");
            }

            // Copy debug messages
            else if (sender == menuCopyDebug)
            {
                string clipboard = "";
                List<string> rows;
                driver.ConsoleLock();
                rows = SearchRows(driver.ConsoleBuffer, "[DEBUG]", 0, 0);
                driver.ConsoleUnlock();
                foreach (string row in rows)
                    clipboard += row + "\r\n";
                Clipboard.SetText(clipboard);
                SetStatus("Debug message copied to clipboard");
            }

            // Copy error messages
            else if (sender == menuCopyErrors)
            {
                string clipboard = "";
                List<string> rows;
                driver.ConsoleLock();
                rows = SearchRows(driver.ConsoleBuffer, "[ERROR]", 1, 1);
                driver.ConsoleUnlock();
                foreach (string row in rows)
                    clipboard += row + "\r\n";
                Clipboard.SetText(clipboard);
                SetStatus("Error message copied to clipboard");
            }

            // Start debug log
            else if (sender == menuStartDebug)
            {
                string logFilename = "debug_" + DateTime.Now.ToString("yyyy-MM-dd_HH_mm_ss") + ".txt";
                ConsoleSendCommand("log " + logFilename);
                ConsoleSendCommand("debug 1");
            }

            // Stop debug log
            else if (sender == menuStopDebug)
            {
                ConsoleSendCommand("log off");
                ConsoleSendCommand("debug 0");
            }

            // Show latest debug log
            else if (sender == menuFindLatestDebugLog)
            {
                try
                {
                    var files = Directory.GetFiles(".", "debug_*.txt").OrderBy(a => File.GetCreationTime(a));
                    if (files.Count() > 0)
                    {
                        string file = files.Last().ToString();
                        Process.Start("explorer.exe", "/n /e,/select," + file);
                    }
                }
                catch (Exception)
                {
                }
            }

            // Run benchmark
            else if (sender == menuRunBenchmark)
            {
                ConsoleSendCommand("Benchmark");
            }

            // Copy Benchmark
            else if (sender == menuCopyBenchmark)
            {
                string clipboard = "";
                List<string> rows;
                driver.ConsoleLock();
                rows = SearchRows(driver.ConsoleBuffer, " [STATUS] BENCHMARK ", 0, 0);
                driver.ConsoleUnlock();
                foreach (string row in rows)
                {
                    Match m = Regex.Match(row, " BENCHMARK ([0-9\\.]+) ([0-9\\.]+) ([0-9\\.]+) (.*)$");
                    if (m.Success)
                    {
                        string tabletName = m.Groups[4].ToString();
                        string totalReports = m.Groups[1].ToString();
                        string noiseWidth = m.Groups[2].ToString();
                        string noiseHeight = m.Groups[3].ToString();
                        clipboard =
                            "Tablet(" + tabletName + ") " +
                            "Noise(" + noiseWidth + " mm x " + noiseHeight + " mm) " +
                            "Reports(" + totalReports + ")\r\n";
                    }
                }

                if (clipboard.Length > 0)
                {
                    Clipboard.SetText(clipboard);
                    SetStatus("Benchmark result copied to clipboard");
                }
            }


            // Measure 2 points
            else if (sender == menuMeasure2)
            {
                ConsoleSendCommand("Measure 2");
            }

            // Open startup log
            else if (sender == menuFindStartupLog)
            {
                if (File.Exists("startuplog.txt"))
                {
                    try { Process.Start("explorer.exe", "/n /e,/select,startuplog.txt"); } catch (Exception) { }
                }
                else
                {
                    MessageBox.Show(
                        "Startup log not found!\n" +
                        "Make sure that it is possible to create and edit files in the '" + Directory.GetCurrentDirectory() + "' directory.\n",
                        "Error!", MessageBoxButton.OK, MessageBoxImage.Error
                    );
                }
            }

            // Open driver folder
            else if (sender == menuOpenFolder)
            {
                try { Process.Start("."); } catch (Exception) { }
            }

            // Open GitHub page
            else if (sender == menuOpenGithub)
            {
                try { Process.Start("https://github.com/hawku/TabletDriver"); } catch (Exception) { }
            }

            // Open Latest URL
            else if (sender == menuOpenLatestURL)
            {
                Regex regex = new Regex("(http[s]?://.+?)($|\\s)", RegexOptions.IgnoreCase | RegexOptions.Multiline);
                MatchCollection matches = regex.Matches(textConsole.Text);
                if (matches.Count > 0)
                {
                    string url = matches[matches.Count - 1].Groups[0].ToString().Trim();
                    try { Process.Start(url); } catch (Exception) { }
                }
            }

            // Report a problem
            else if (sender == menuReportProblem)
            {
                try { Process.Start("https://github.com/hawku/TabletDriver/wiki/FAQ"); } catch (Exception) { }
            }


        }


    }
}
