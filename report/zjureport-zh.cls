\ProvidesClass{zjureport-zh}
\LoadClass{ctexart}
%\RequirePackage{xeCJK}
\RequirePackage{amsmath}
\RequirePackage{amsthm}
\RequirePackage{tikz}
\RequirePackage{bm}

\RequirePackage{titlesec}
\RequirePackage{geometry}
\RequirePackage{graphicx}
\RequirePackage{listings}
\usepackage[perpage,bottom]{footmisc}
\RequirePackage{enumerate}
\RequirePackage{enumitem}
%\RequirePackage{xkeyval}

\RequirePackage[colorlinks,urlcolor=blue]{hyperref}
\RequirePackage{fontspec}

\RequirePackage{multirow}
\RequirePackage{multicol}
\RequirePackage{makecell}
\RequirePackage{subfigure}
%\RequirePackage{caption}


% 页面
\pagestyle{plain}

% 页边距
\geometry{top=1in,bottom=1in,left=1in,right=1in}

% 字体
\setmonofont{Menlo}
\newCJKfontfamily[kai]\kaiti{STKaiti}

% tikz
\tikzset{global scale/.style={
    scale=#1,
    every node/.append style={scale=#1}
  }
}

% 标题
\titleformat*{\section}{\large\bfseries}
\titleformat*{\subsection}{\normalsize\bfseries}
\titleformat*{\subsubsection}{\normalsize\kaiti}

% 脚注
\newcommand{\myfn}[1]{\text{$^[$}\footnote{#1}\text{$^]$}}
\newcommand{\myfnmark}{\text{$^[$}\footnotemark\text{$^]$}}

% 信息
\newcommand\major[1]{\def\@major{#1}}
\newcommand\name[1]{\def\@name{#1}}
\newcommand\stuid[1]{\def\@stuid{#1}}
\newcommand\college[1]{\def\@college{#1}}
\renewcommand\date[1]{\def\@date{#1}}
\newcommand\lab[1]{\def\@lab{#1}}
\newcommand\course[1]{\def\@course{#1}}
\newcommand\instructor[1]{\def\@instructor{#1}}
\newcommand\grades[1]{\def\@grades{#1}}
\newcommand\expname[1]{\def\@expname{#1}}
\newcommand\exptype[1]{\def\@exptype{#1}}
\newcommand\partner[1]{\def\@partner{#1}}

% 目录
\renewcommand\contentsname{\textbf{目录}}

% 图表
\renewcommand\figurename{图}
\renewcommand\tablename{表}
\renewcommand{\thefigure}{\arabic{section}.\arabic{figure}}
\renewcommand{\thetable}{\arabic{section}.\arabic{table}}
\newcommand{\figref}[1]{图 \ref{#1}}
\newcommand{\tabref}[1]{表 \ref{#1}}

% 封面
\newcommand{\makecover}{
	\makeatletter
	\begin{center}
		\begin{center}
			\includegraphics[width=0.8\linewidth]{./imgs/cover.png}
		\end{center}
		\vspace{1cm}
		\textbf{\huge{本科生课程作业}} \\
		\vspace{0.5cm}
		\textbf{\Large{\@expname}}
		\vspace{3cm}
		\begin{center}
			\Large
			\linespread{2}
			\centering
			\renewcommand\arraystretch{1.6}
			\begin{tabular}{p{3cm}p{6cm}}
				\textbf{课程名称：} & \@course \\ \cline{2-2}
				\textbf{姓名：}     & \@name   \\ \cline{2-2}
				\textbf{学院：}     & \@college \\ \cline{2-2}
				\textbf{专业：}     & \@major  \\ \cline{2-2}
				\textbf{学号：}     & \@stuid  \\ \cline{2-2}
				\textbf{指导老师：} & \@instructor  \\ \cline{2-2}
			\end{tabular}
		\end{center}
		\vspace{2cm}
		\textbf{\Large{\@date}}
	\end{center}
	\makeatother
	\thispagestyle{empty}
	\newpage
}

% 标题
\renewcommand{\maketitle}{
	\huge
	\begin{center}
		\textbf{\@title}
	\end{center}
	\thispagestyle{empty}
}

% 代码格式和颜色定义
\newcommand{\code}[1]{\m}
\definecolor{dkgreen}{rgb}{0,0.6,0}
\definecolor{gray}{rgb}{0.5,0.5,0.5}
\definecolor{comment}{rgb}{0.56,0.64,0.68}
\lstset{
	frame=tb,
	aboveskip=3mm,
	belowskip=3mm,
	xleftmargin=1em,
	xrightmargin=1em,
	showstringspaces=false,
	columns=flexible,
	framerule=1pt,
	rulecolor=\color{gray!35},
	backgroundcolor=\color{gray!5},
	basicstyle={\small\ttfamily},
	numbers=left,
	numberstyle=\tiny\color{gray},
	keywordstyle=\color{blue},
	commentstyle=\color{comment},
	stringstyle=\color{dkgreen},
	breaklines=true,
	breakatwhitespace=true,
	tabsize=2,
}