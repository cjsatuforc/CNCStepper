﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Roboter.Wpf
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }
/*
        private void Button_ConnectClick(object sender, RoutedEventArgs e)
        {
            if (Com.IsConnected)
            {
            }
            else 
            {
                Com.Connect("com1");
            }
        }
*/
        private void Button_ManualControlClick(object sender, RoutedEventArgs e)
        {
             new ManualControl().ShowDialog();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            Framework.Tools.Singleton<Roboter.Logic.Communication>.Free();
        }
    }
}