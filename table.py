import numpy as np
from typing import Union

# [('', []), ('', []), ('', []), ...]
def _list_as_length(l:list, n):
	if l:
		length = len(l)
		if length > n:
			ret = l[:n]
		else:
			ret = l
			if length < n:
				ret += [None for _ in range(n-length)]
	else:
		ret = [None for _ in range(n)]
	return ret


class Table:
	def __init__(self, n_row, n_col, caption:str=None):
		self._caption = caption
		assert n_row > 0 and n_col > 0
		self._n_row = n_row
		self._n_col = n_col
		self.row_header = None
		self.col_header = None
		self.data = None
		self.none_string = ''
		self.float_precision = 4

	@property
	def n_row(self):
		return self._n_row
	
	@property
	def n_col(self):
		return self._n_col
	
	@property
	def shape(self):
		return self._n_row, self._n_col
	
	@property
	def none_string(self):
		return self._none_string
	
	@none_string.setter
	def none_string(self, s:str):
		self._none_string = s if s else ''
	
	@property
	def float_precision(self):
		return self._float_precision
	
	@float_precision.setter
	def float_precision(self, n:int):
		self._float_precision = n if n > 0 else 0
	
	@property
	def row_header(self):
		return self._row_header
	
	@row_header.setter
	def row_header(self, row_header:list=None):
		self._row_header = _list_as_length(row_header, self.n_row)
	
	@property
	def col_header(self):
		return self._col_header
	
	@col_header.setter
	def col_header(self, col_header:list=None):
		self._col_header = _list_as_length(col_header, self.n_col+1)
	
	@property
	def data(self):
		return self._data
	
	@data.setter
	def data(self, data:Union[list, np.ndarray]=None):
		if isinstance(data, np.ndarray):
			assert data.shape == self.shape
			self._data = data
		else:
			self._data = _list_as_length(data, self.n_row)
			if self._data:
				self._data = [_list_as_length(row, self.n_col) for row in self._data]
	
	def string(self, s):
		if isinstance(s, float):
			s = str(round(s, self.float_precision))
		return str(s) if s is not None else self.none_string

	def to_latex(self):
		ret = '\\begin{table}[h]\n'
		ret += '\\centering\n'
		ret += '\\caption{' + self.string(self._caption) + '}\n'
		ret += '\\vspace{1ex}\n'
		
		ret += '\\begin{tabular}{|'
		for _ in range(self.n_col+1):
			ret += 'c|' 
		ret += '}\n'

		ret += '\t\\hline\n'
		for i in range(self.n_row+1):
			ret += '\t'
			if i == 0:
				for j in range(self.n_col+1):
					if j > 0:
						ret += '& '
					ret += self.string(self.col_header[j])
					ret += ' '
			else:
				ret += self.string(self.row_header[i-1])
				ret += ' '
				for d in self.data[i-1]:
					ret += '& '
					ret += self.string(d)
					ret += ' '
			ret += '\\\\\n'
			ret += '\t\\hline\n'

		ret += '\\end{tabular}\n'
		ret += '\\end{table}\n'
		return ret

