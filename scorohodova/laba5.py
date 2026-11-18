"""
Лабораторная работа №5. Табличный симплекс-метод и метод искусственных переменных (M-метод).

Задача ЛП с двумя переменными:  W = c1*X1 + c2*X2 -> min / max
Ограничения:                    a1*X1 + a2*X2  (<=, >=, =)  b ;   X1, X2 >= 0

Расчёты ведутся в точных дробях (fractions.Fraction), поэтому в таблицах нет
ошибок округления. Запуск:  python simplex_lab5.py
"""

from fractions import Fraction as F
import sys
import tkinter as tk
import tkinter.font as tkfont
from tkinter import ttk, messagebox


# ======================================================================
#                              РЕШАТЕЛЬ
# ======================================================================
def fmt(x):
    """Дробь -> строка: 3, -1/6, 7/8."""
    x = F(x)
    return str(x.numerator) if x.denominator == 1 else f"{x.numerator}/{x.denominator}"


def ratio_str(b, a):
    """Строка отношения B/a для столбца-подсказки, как в рукописных таблицах."""

    def w(v):
        t = fmt(v)
        return f"({t})" if ("/" in t or v < 0) else t

    return f"{w(b)}/{w(a)}"


def solve(c, constraints, goal="min"):
    """
    c           -- [c1, c2] коэффициенты целевой функции
    constraints -- список (a1, a2, знак, b), знак из {"<=", ">=", "="}
    goal        -- "min" или "max"

    Возвращает словарь:
      iterations -- список таблиц (каждая -- словарь для отрисовки)
      status     -- "optimal" | "unbounded" | "infeasible"
      x, W       -- оптимальный план и значение целевой функции (если optimal)
    Для max решается задача на min для -W, ответ пересчитывается обратно.
    """
    sign_goal = 1 if goal == "min" else -1
    cost = [F(v) * sign_goal for v in c]  # внутри всегда MIN
    m = len(constraints)

    rows = []  # (a1, a2, знак, b) с b >= 0
    for a1, a2, s, b in constraints:
        a1, a2, b = F(a1), F(a2), F(b)
        if b < 0:  # умножаем на -1
            a1, a2, b = -a1, -a2, -b
            s = {"<=": ">=", ">=": "<=", "=": "="}[s]
        rows.append((a1, a2, s, b))

    # --- приведение к каноническому виду ---
    # столбцы: X1, X2, затем доп. переменные (X3...), затем искусственные
    n_main = 2
    extra = []  # (номер ограничения, +1 / -1)
    for i, (_, _, s, _) in enumerate(rows):
        if s == "<=":
            extra.append((i, +1))
        elif s == ">=":
            extra.append((i, -1))
    n_extra = len(extra)
    art_rows = [i for i, (_, _, s, _) in enumerate(rows) if s in (">=", "=")]
    n = n_main + n_extra + len(art_rows)

    A = [[F(0)] * n for _ in range(m)]
    b = [r[3] for r in rows]
    for i, (a1, a2, _, _) in enumerate(rows):
        A[i][0], A[i][1] = a1, a2
    for k, (i, sg) in enumerate(extra):
        A[i][n_main + k] = F(sg)
    basis = [None] * m
    for k, i in enumerate(art_rows):
        A[i][n_main + n_extra + k] = F(1)
        basis[i] = n_main + n_extra + k
    for k, (i, sg) in enumerate(extra):
        if sg == +1:
            basis[i] = n_main + k  # добавочная переменная даёт единичный столбец

    art = set(range(n_main + n_extra, n))
    c_full = cost + [F(0)] * n_extra + [None] * len(art)  # None == M

    iterations, k = [], 0
    while True:
        has_art = any(v in art for v in basis)

        # --- оценки: Δj = Σ cБ*aij - cj ; отдельно коэффициент при M и обычная часть ---
        def cb(i):
            return (1, F(0)) if basis[i] in art else (0, c_full[basis[i]])

        W_row, M_row = [], []
        for j in range(n):
            zM = sum(cb(i)[0] * A[i][j] for i in range(m))
            zc = sum(cb(i)[1] * A[i][j] for i in range(m))
            cjM, cjc = (1, F(0)) if j in art else (0, c_full[j])
            M_row.append(zM - cjM)
            W_row.append(zc - cjc)
        Wb = sum(cb(i)[1] * b[i] for i in range(m))
        Mb = sum(cb(i)[0] * b[i] for i in range(m))

        # --- выбор вводимого вектора ---
        enter = None
        if has_art:
            cand = [(M_row[j], -j) for j in range(n) if M_row[j] > 0 and j not in art]
            if cand:
                enter = -max(cand)[1]
        stuck = (
            has_art
            and enter is None
            and any(basis[i] in art and b[i] != 0 for i in range(m))
        )
        if enter is None and not stuck:
            cand = [(W_row[j], -j) for j in range(n) if W_row[j] > 0 and j not in art]
            if cand:
                enter = -max(cand)[1]

        table = dict(
            k=k,
            basis=list(basis),
            cB=["M" if v in art else fmt(c_full[v]) for v in basis],
            A=[[fmt(v) for v in r] for r in A],
            b=[fmt(v) for v in b],
            W=[fmt(v) for v in W_row],
            Wb=fmt(Wb),
            M=[fmt(v) for v in M_row],
            Mb=fmt(Mb),
            has_art=has_art,
            n=n,
            art=sorted(art),
            c_head=[("M" if j in art else fmt(c_full[j])) for j in range(n)],
            enter=None,
            leave=None,
            ratio=None,
            message="",
        )

        if enter is None:  # оптимум M-задачи
            iterations.append(table)
            if has_art and any(basis[i] in art and b[i] != 0 for i in range(m)):
                table["message"] = (
                    "В оптимальном плане есть ненулевая искусственная переменная -> система ограничений несовместна."
                )
                return dict(iterations=iterations, status="infeasible")
            x = [F(0)] * n
            for i, v in enumerate(basis):
                x[v] = b[i]
            W_val = sum(cost[j] * x[j] for j in range(2)) * sign_goal
            table["message"] = (
                "Коэффициенты во всех оценках ≤ 0, значит оптимальное решение найдено."
            )
            return dict(
                iterations=iterations,
                status="optimal",
                x=x[: n_main + n_extra],
                W=W_val,
            )

        # --- выбор выводимого вектора (min B/a при a > 0) ---
        cand = [(b[i] / A[i][enter], i) for i in range(m) if A[i][enter] > 0]
        if not cand:
            table["enter"] = enter
            table["message"] = (
                "В направляющем столбце нет положительных элементов -> целевая функция не ограничена."
            )
            iterations.append(table)
            return dict(iterations=iterations, status="unbounded")
        leave = min(cand)[1]
        table.update(
            enter=enter,
            leave=leave,
            ratio=[ratio_str(b[i], A[i][enter]) for i in range(m)],
            message=f"Минимальное отношение -> выводим X{basis[leave] + 1}",
        )
        iterations.append(table)

        # --- жорданово преобразование ---
        p = A[leave][enter]
        A[leave] = [v / p for v in A[leave]]
        b[leave] /= p
        for i in range(m):
            if i != leave:
                f = A[i][enter]
                A[i] = [a - f * r for a, r in zip(A[i], A[leave])]
                b[i] -= f * b[leave]
        basis[leave] = enter
        k += 1


# ======================================================================
#                              ИНТЕРФЕЙС
# ======================================================================
BG_HEAD, BG_RATIO, BG_COL, BG_PIV, BG_OK = (
    "#dce4f3",
    "#fff2cc",
    "#fbe5d6",
    "#ffd966",
    "#b7d7a8",
)
MONO = MONO_B = None  # шрифты создаются в App.__init__ (после создания окна)


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Симплекс-метод и метод искусственных переменных")
        self._init_scale_and_fonts()
        self.rows = []  # строки ввода ограничений
        self._build_input()
        self._build_output()
        # по умолчанию все поля заполнены нулями
        self.goal.set("min")
        self.c1.insert(0, "0")
        self.c2.insert(0, "0")
        for _ in range(3):
            self.add_row(0, 0, "<=", 0)
        # масштаб: Ctrl + плюс / Ctrl + минус / Ctrl + 0
        for k in ("<Control-plus>", "<Control-equal>", "<Control-KP_Add>"):
            self.bind(k, lambda e: self.set_scale(self.scale * 1.1))
        for k in ("<Control-minus>", "<Control-KP_Subtract>"):
            self.bind(k, lambda e: self.set_scale(self.scale / 1.1))
        self.bind("<Control-Key-0>", lambda e: self.set_scale(self.auto_scale))

    # ---------- масштаб ----------
    def _init_scale_and_fonts(self):
        """Масштаб: по ширине экрана (4K -> x2). Можно задать вручную: python simplex_lab5.py --scale 2.5"""
        sw, sh = self.winfo_screenwidth(), self.winfo_screenheight()
        self.auto_scale = max(1.0, sw / 1920)
        self.scale = self.auto_scale
        if "--scale" in sys.argv:
            try:
                self.scale = float(sys.argv[sys.argv.index("--scale") + 1])
            except (ValueError, IndexError):
                pass
        self.geometry(
            f"{int(sw * 0.75)}x{int(sh * 0.85)}+{int(sw * 0.1)}+{int(sh * 0.05)}"
        )
        global MONO, MONO_B
        self.f_ui = tkfont.Font(family="Arial", size=-16)
        self.f_ui_b = tkfont.Font(family="Arial", size=-16, weight="bold")
        self.f_big = tkfont.Font(family="Arial", size=-22, weight="bold")
        self.f_res = tkfont.Font(family="Arial", size=-20, weight="bold")
        MONO = tkfont.Font(family="Courier New", size=-17)
        MONO_B = tkfont.Font(family="Courier New", size=-17, weight="bold")
        self._font_base = [
            (self.f_ui, 16),
            (self.f_ui_b, 16),
            (self.f_big, 22),
            (self.f_res, 20),
            (MONO, 17),
            (MONO_B, 17),
        ]
        self.tk_defaults = [
            tkfont.nametofont(n) for n in ("TkDefaultFont", "TkTextFont", "TkMenuFont")
        ]
        self.set_scale(self.scale)

    def set_scale(self, value):
        self.scale = max(0.6, min(value, 6.0))
        for f, base in self._font_base:
            f.configure(size=-int(round(base * self.scale)))
        for f in self.tk_defaults:  # поля ввода, выпадающие списки, кнопки
            f.configure(size=-int(round(16 * self.scale)))
        style = ttk.Style(self)
        style.configure("TCombobox", arrowsize=int(16 * self.scale))
        style.configure("Vertical.TScrollbar", arrowsize=int(16 * self.scale))
        self.option_add("*TCombobox*Listbox.font", self.f_ui)

    # ---------- ввод ----------
    def _build_input(self):
        top = tk.Frame(self, pady=8)
        top.pack(fill="x")
        line = tk.Frame(top)
        line.pack()
        tk.Label(line, text="W =", font=self.f_big).pack(side="left")
        self.c1 = tk.Entry(line, width=6, justify="center")
        self.c1.pack(side="left", padx=4)
        tk.Label(line, text="X1 +").pack(side="left")
        self.c2 = tk.Entry(line, width=6, justify="center")
        self.c2.pack(side="left", padx=4)
        tk.Label(line, text="X2   ->").pack(side="left")
        self.goal = tk.StringVar(value="min")
        ttk.Combobox(
            line,
            textvariable=self.goal,
            values=["min", "max"],
            width=5,
            state="readonly",
        ).pack(side="left", padx=6)

        tk.Label(top, text="Ограничения:", font=self.f_ui_b).pack(pady=(10, 2))
        self.cons_frame = tk.Frame(top)
        self.cons_frame.pack()
        btns = tk.Frame(top)
        btns.pack(pady=6)
        tk.Button(btns, text="+ ограничение", command=self.add_row).pack(
            side="left", padx=4
        )
        tk.Button(btns, text="- ограничение", command=self.remove_row).pack(
            side="left", padx=4
        )
        tk.Button(
            btns, text="Решить", command=self.on_solve, bg="#c6e0b4", width=12
        ).pack(side="left", padx=12)
        tk.Button(btns, text="Очистить", command=self.clear_output).pack(
            side="left", padx=4
        )

    def add_row(self, a1="", a2="", s="<=", b=""):
        fr = tk.Frame(self.cons_frame)
        fr.pack(pady=2)
        e1 = tk.Entry(fr, width=6, justify="center")
        e1.pack(side="left")
        e1.insert(0, str(a1))
        tk.Label(fr, text="X1 +").pack(side="left")
        e2 = tk.Entry(fr, width=6, justify="center")
        e2.pack(side="left", padx=4)
        e2.insert(0, str(a2))
        tk.Label(fr, text="X2").pack(side="left")
        sv = tk.StringVar(value=s)
        ttk.Combobox(
            fr, textvariable=sv, values=["<=", ">=", "="], width=3, state="readonly"
        ).pack(side="left", padx=6)
        e3 = tk.Entry(fr, width=6, justify="center")
        e3.pack(side="left")
        e3.insert(0, str(b))
        self.rows.append((fr, e1, e2, sv, e3))

    def remove_row(self):
        if len(self.rows) > 1:
            self.rows.pop()[0].destroy()

    # ---------- вывод ----------
    def _build_output(self):
        wrap = tk.Frame(self)
        wrap.pack(fill="both", expand=True)
        self.canvas = tk.Canvas(wrap, highlightthickness=0)
        sb = ttk.Scrollbar(wrap, orient="vertical", command=self.canvas.yview)
        self.canvas.configure(yscrollcommand=sb.set)
        sb.pack(side="right", fill="y")
        self.canvas.pack(side="left", fill="both", expand=True)
        self.out = tk.Frame(self.canvas)
        self.canvas.create_window((0, 0), window=self.out, anchor="n")
        self.out.bind(
            "<Configure>",
            lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all")),
        )
        self.canvas.bind_all(
            "<MouseWheel>",
            lambda e: self.canvas.yview_scroll(int(-e.delta / 120), "units"),
        )

    def clear_output(self):
        for w in self.out.winfo_children():
            w.destroy()

    def _read(self):
        try:
            c = [
                F(self.c1.get().replace(",", ".").strip()),
                F(self.c2.get().replace(",", ".").strip()),
            ]
            cons = []
            for _, e1, e2, sv, e3 in self.rows:
                cons.append(
                    (
                        F(e1.get().replace(",", ".").strip()),
                        F(e2.get().replace(",", ".").strip()),
                        sv.get(),
                        F(e3.get().replace(",", ".").strip()),
                    )
                )
        except (ValueError, ZeroDivisionError):
            raise ValueError(
                "Все коэффициенты должны быть числами (допустимы дроби вида 1/3 и десятичные 0.5)."
            )
        return c, cons

    def on_solve(self):
        self.clear_output()
        try:
            c, cons = self._read()
        except ValueError as e:
            messagebox.showerror("Ошибка ввода", str(e))
            return
        res = solve(c, cons, self.goal.get())
        for it in res["iterations"]:
            self.draw_table(it)
        if res["status"] == "optimal":
            x = res["x"]
            names = "   ".join(f"X{i + 1} = {fmt(v)}" for i, v in enumerate(x))
            txt = f"Оптимальное решение найдено.\n{names}\nW = {fmt(res['W'])}"
            color = "#1e6b34"
        elif res["status"] == "unbounded":
            txt, color = "Целевая функция не ограничена.", "#c00000"
        else:
            txt, color = "Система ограничений несовместна.", "#c00000"
        tk.Label(self.out, text=txt, fg=color, font=self.f_res, pady=10).pack()
        self.canvas.yview_moveto(0)

    def draw_table(self, it):
        card = tk.Frame(self.out, bd=1, relief="solid", padx=10, pady=8)
        card.pack(pady=8, padx=20)
        tk.Label(card, text=f"Итерация {it['k']}", font=self.f_res).pack()
        grid = tk.Frame(card)
        grid.pack(pady=6)
        final = it["enter"] is None
        # при отсутствии искусственных переменных в базисе их столбцы и M-строка скрываются
        cols = [j for j in range(it["n"]) if it["has_art"] or j not in it["art"]]
        show_M = it["has_art"]

        def cell(r, c, text, bg=None, font=MONO, width=7):
            tk.Label(
                grid,
                text=text,
                font=font,
                width=width,
                bg=bg or "white",
                relief="solid",
                bd=1,
            ).grid(row=r, column=c, sticky="nsew")

        r = 0
        cell(r, 0, "C", BG_HEAD, MONO_B)
        cell(r, 1, "", BG_HEAD)
        cell(r, 2, "", BG_HEAD)
        for ci, j in enumerate(cols):
            cell(r, 3 + ci, it["c_head"][j], BG_HEAD, MONO_B)
        r += 1
        cell(r, 0, "", BG_HEAD)
        cell(r, 1, "X", BG_HEAD, MONO_B)
        cell(r, 2, "B", BG_HEAD, MONO_B)
        for ci, j in enumerate(cols):
            cell(r, 3 + ci, f"A{j + 1}", BG_HEAD, MONO_B)
        rc_col = 3 + len(cols)
        if not final:
            cell(r, rc_col, f"B/A{it['enter'] + 1}", BG_RATIO, MONO_B, 14)
        r += 1
        for i, v in enumerate(it["basis"]):
            cell(r, 0, it["cB"][i])
            cell(r, 1, f"X{v + 1}", font=MONO_B)
            cell(
                r,
                2,
                it["b"][i],
                BG_OK if (final and it["k"] >= 0 and not show_M) else None,
                MONO_B,
            )
            for ci, j in enumerate(cols):
                bg, f = None, MONO
                if not final and j == it["enter"]:
                    bg = BG_PIV if i == it["leave"] else BG_COL
                    f = MONO_B if i == it["leave"] else MONO
                cell(r, 3 + ci, it["A"][i][j], bg, f)
            if not final:
                cell(
                    r,
                    rc_col,
                    it["ratio"][i],
                    BG_PIV if i == it["leave"] else BG_RATIO,
                    MONO,
                    14,
                )
            r += 1
        if show_M:
            cell(r, 0, "W", BG_HEAD, MONO_B)
            cell(r, 1, "M", BG_HEAD, MONO_B)
            cell(r, 2, it["Mb"], BG_HEAD, MONO_B)
            for ci, j in enumerate(cols):
                cell(r, 3 + ci, it["M"][j], BG_HEAD, MONO_B)
            r += 1
            cell(r, 0, "", BG_HEAD)
            cell(r, 1, "", BG_HEAD)
        else:
            cell(r, 0, "W", BG_HEAD, MONO_B)
            cell(r, 1, "", BG_HEAD)
        cell(r, 2, it["Wb"], BG_OK if final else BG_HEAD, MONO_B)
        for ci, j in enumerate(cols):
            cell(r, 3 + ci, it["W"][j], BG_HEAD, MONO_B)
        color = "#b8860b" if not final else "#000000"
        tk.Label(
            card,
            text=it["message"],
            fg=color,
            font=self.f_ui_b,
            wraplength=int(900 * self.scale),
        ).pack()


if __name__ == "__main__":
    App().mainloop()
