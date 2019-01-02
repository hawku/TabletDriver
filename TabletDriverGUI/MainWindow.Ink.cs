using Microsoft.Win32;
using System;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace TabletDriverGUI
{
    public partial class MainWindow : Window
    {

        // Ink canvas undo history
        StrokeCollection inkCanvasUndoHistory;

        // Ink canvas DrawingAttributes
        DrawingAttributes inkCanvasDrawingAttributes;


        //
        // Ink canvas stylys move
        //
        private void InkCanvas_StylusMove(object sender, StylusEventArgs e)
        {
            double pressure = 0;
            int count = 0;
            StylusPointCollection points = e.GetStylusPoints(inkCanvas);
            foreach (var point in points)
            {
                pressure += point.PressureFactor;
                count++;
            }
            progressPressure.Value = pressure / count;
        }


        //
        // Ink canvas stylus up
        //
        private void InkCanvas_StylusUp(object sender, StylusEventArgs e)
        {
            progressPressure.Value = 0;

            Random random = new Random();
            double shade = random.Next(0x33, 0x77);
            inkCanvasDrawingAttributes.Color = Color.FromRgb(
                (byte)(shade * (0.95 + random.NextDouble() * 0.1)),
                (byte)(shade * (0.95 + random.NextDouble() * 0.1)),
                (byte)(shade * (0.95 + random.NextDouble() * 0.1))
            );
            if (inkCanvasUndoHistory != null && inkCanvasUndoHistory.Count > 0)
            {
                inkCanvasUndoHistory.Clear();
            }
        }


        //
        // Ink canvas key down
        //
        private void InkCanvas_KeyDown(object sender, KeyEventArgs e)
        {
            // Ctrl + Z undo
            if (e.Key == Key.Z && e.KeyboardDevice.Modifiers == ModifierKeys.Control)
            {

                ButtonInkUndo_Click(sender, null);
            }

            // Ctrl + Y redo
            if (e.Key == Key.Y && e.KeyboardDevice.Modifiers == ModifierKeys.Control)
            {
                ButtonInkRedo_Click(sender, null);
            }

        }


        //
        // Windows Ink pressure sensitivity changed
        //
        private void SliderPressure_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            sliderPressureSensitivity.ToolTip = Utils.GetNumberString(-sliderPressureSensitivity.Value);
            sliderPressureDeadzoneLow.ToolTip = Utils.GetNumberString(sliderPressureDeadzoneLow.Value * 100) + "%";
            sliderPressureDeadzoneHigh.ToolTip = Utils.GetNumberString(sliderPressureDeadzoneHigh.Value * 100) + "%";

            if (isLoadingSettings) return;

            config.PressureSensitivity = sliderPressureSensitivity.Value;
            config.PressureDeadzoneLow = sliderPressureDeadzoneLow.Value;
            config.PressureDeadzoneHigh = sliderPressureDeadzoneHigh.Value;


            driver.SendCommand("PressureSensitivity " + Utils.GetNumberString(config.PressureSensitivity));
            driver.SendCommand("PressureDeadzone " +
                Utils.GetNumberString(config.PressureDeadzoneLow) + " " +
                Utils.GetNumberString(config.PressureDeadzoneHigh)
            );
        }


        //
        // Clear ink canvas
        //
        private void ButtonInkClear_Click(object sender, RoutedEventArgs e)
        {
            inkCanvas.Strokes.Clear();
            inkCanvasUndoHistory.Clear();
        }


        //
        // Undo ink canvas
        //
        private void ButtonInkUndo_Click(object sender, RoutedEventArgs e)
        {
            if (inkCanvas.Strokes.Count > 0)
            {
                inkCanvasUndoHistory.Add(inkCanvas.Strokes[inkCanvas.Strokes.Count - 1]);
                inkCanvas.Strokes.RemoveAt(inkCanvas.Strokes.Count - 1);
            }
        }


        //
        // Redo ink canvas
        //
        private void ButtonInkRedo_Click(object sender, RoutedEventArgs e)
        {
            if (inkCanvasUndoHistory.Count > 0)
            {
                inkCanvas.Strokes.Add(inkCanvasUndoHistory.Last());
                inkCanvasUndoHistory.RemoveAt(inkCanvasUndoHistory.Count - 1);
            }
        }


        //
        // Save ink canvas
        //
        private void ButtonInkSave_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog fileDialog = new SaveFileDialog
            {
                DefaultExt = ".png",
                Filter = "PNG Files (*.png)|*.png",
                FileName = "inktest_" + DateTime.Now.ToString("yyyy-MM-dd_HHmmss") + ".png"
            };

            // File selection OK?
            if (fileDialog.ShowDialog() == true)
            {
                try
                {
                    RenderInkCanvasToPNG(inkCanvas, 2.0, fileDialog.FileName);
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Saving failed!\n" + ex.Message, "ERROR!", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }


        //
        // Render Ink canvas to PNG
        //
        private void RenderInkCanvasToPNG(InkCanvas canvas, double scale, string filepath)
        {
            double inkWidth = canvas.ActualWidth * scale;
            double inkHeight = canvas.ActualHeight * scale;

            // Draw ink canvas to drawing visual
            DrawingVisual drawingVisual = new DrawingVisual();
            using (DrawingContext context = drawingVisual.RenderOpen())
            {
                VisualBrush visualBrush = new VisualBrush(canvas);
                Brush borderBrush = new SolidColorBrush(inkCanvas.DefaultDrawingAttributes.Color);
                Pen borderPen = new Pen(borderBrush, 5);
                context.DrawRectangle(visualBrush, borderPen, new Rect(0, 0, inkWidth, inkHeight));
            }

            // Render drawing visual to a bitmap
            RenderTargetBitmap targetBitmap = new RenderTargetBitmap(
                (int)inkWidth,
                (int)inkHeight,
                96, 96,
               PixelFormats.Default
            );
            targetBitmap.Render(drawingVisual);

            // Encode bitmap to PNG
            PngBitmapEncoder encoder = new PngBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(targetBitmap));

            // Save PNG to a file
            FileStream fileStream = File.Open(filepath, FileMode.OpenOrCreate);
            encoder.Save(fileStream);

            // Close the file
            fileStream.Close();

            encoder = null;
            targetBitmap = null;
            drawingVisual = null;
        }

    }
}
